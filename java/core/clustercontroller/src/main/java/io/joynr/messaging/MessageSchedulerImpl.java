package io.joynr.messaging;

/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
 * %%
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * #L%
 */

import io.joynr.exceptions.JoynrSendBufferFullException;
import io.joynr.exceptions.JoynrShutdownException;
import io.joynr.messaging.http.HttpMessageSender;

import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.inject.Inject;
import com.google.inject.name.Named;

/**
 * The MessageScheduler queues message post requests in a single threaded executor. The executor is blocked until the
 * connection is established, from there on the request is async. If there are already too much connections open, the
 * executor is blocked until one of the connections is closed. Resend attempts are scheduled by a cached thread pool
 * executor.
 */
@edu.umd.cs.findbugs.annotations.SuppressWarnings(value = "JLM_JSR166_UTILCONCURRENT_MONITORENTER", justification = "ensure that no new messages are scheduled when scheduler is shuting down")
public class MessageSchedulerImpl implements MessageScheduler {
    private static final long TERMINATION_TIMEOUT = 5000;
    private static final Logger logger = LoggerFactory.getLogger(MessageSchedulerImpl.class);
    private final HttpMessageSender httpMessageSender;
    private ScheduledExecutorService scheduler;

    @Inject
    public MessageSchedulerImpl(@Named(SCHEDULEDTHREADPOOL) ScheduledExecutorService scheduler,
                                HttpMessageSender httpMessageSender) {
        this.httpMessageSender = httpMessageSender;
        this.scheduler = scheduler;
    }

    /* (non-Javadoc)
     * @see io.joynr.messaging.MessageScheduler#scheduleMessage(io.joynr.messaging.MessageContainer, long, io.joynr.messaging.FailureAction, io.joynr.messaging.MessageReceiver)
     */
    @Override
    public synchronized void scheduleMessage(final MessageContainer messageContainer,
                                             long delayMs,
                                             final FailureAction failureAction) {
        logger.trace("scheduleMessage messageId: {} channelId {}",
                     messageContainer.getMessageId(),
                     messageContainer.getChannelId());

        synchronized (scheduler) {
            if (scheduler.isShutdown()) {
                JoynrShutdownException joynrShutdownEx = new JoynrShutdownException("MessageScheduler is shutting down already. Unable to send message [messageId: "
                        + messageContainer.getMessageId() + "].");
                failureAction.execute(joynrShutdownEx);
                throw joynrShutdownEx;
            }

            try {
                scheduler.schedule(new Runnable() {
                    @Override
                    public void run() {
                        httpMessageSender.sendMessage(messageContainer, failureAction);
                    }
                }, delayMs, TimeUnit.MILLISECONDS);
            } catch (RejectedExecutionException e) {
                logger.error("Execution rejected while scheduling SendSerializedMessageRequest ", e);
                throw new JoynrSendBufferFullException(e);
            }
        }
    }

    /* (non-Javadoc)
     * @see io.joynr.messaging.MessageScheduler#shutdown()
     */
    @Override
    public synchronized void shutdown() throws InterruptedException {
        synchronized (scheduler) {
            scheduler.shutdown();
        }

        // TODO serialize messages that could not be resent because of shutdown? Or somehow notify sender?
        // List<Runnable> awaitingScheduling = scheduler.shutdownNow();
        // List<Runnable> awaitingResend = executionQueue.shutdownNow();
        scheduler.awaitTermination(TERMINATION_TIMEOUT, TimeUnit.MILLISECONDS);
    }

}
