package io.joynr.generator.interfaces

/*
 * !!!
 *
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
 *
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
 */

import com.google.inject.Inject
import io.joynr.generator.templates.InterfaceTemplate
import io.joynr.generator.templates.util.BroadcastUtil
import io.joynr.generator.templates.util.NamingUtil
import io.joynr.generator.util.JavaTypeUtil
import io.joynr.generator.util.JoynrJavaGeneratorExtensions
import io.joynr.generator.util.TemplateBase

class InterfaceBroadcastTemplate extends InterfaceTemplate {
	@Inject extension JoynrJavaGeneratorExtensions
	@Inject extension JavaTypeUtil
	@Inject extension NamingUtil
	@Inject extension BroadcastUtil
	@Inject extension TemplateBase

	override generate() {
		val interfaceName = francaIntf.joynrName
		val broadcastClassName = interfaceName + "BroadcastInterface"
		val packagePath = getPackagePathWithJoynrPrefix(francaIntf, ".")

		'''
«warning()»
package «packagePath»;

import io.joynr.dispatcher.rpc.annotation.JoynrRpcBroadcast;
import io.joynr.dispatcher.rpc.JoynrBroadcastSubscriptionInterface;
import io.joynr.pubsub.subscription.BroadcastSubscriptionListener;
import joynr.OnChangeSubscriptionQos;
import joynr.BroadcastFilterParameters;

«FOR datatype: getRequiredIncludesFor(francaIntf, false, false, false, false, true)»
	import «datatype»;
«ENDFOR»

public interface «broadcastClassName» extends JoynrBroadcastSubscriptionInterface, «interfaceName» {

«FOR broadcast : francaIntf.broadcasts»
	«val broadcastName = broadcast.joynrName»
	«val filterParameters = getFilterParameters(broadcast)»
	«val filterParameterType = broadcastName.toFirstUpper + "BroadcastFilterParameters"»
	«val listenerInterface = broadcastName.toFirstUpper + "BroadcastListener"»

	public interface «listenerInterface» extends BroadcastSubscriptionListener {
		public void onReceive(«broadcast.commaSeperatedTypedOutputParameterList»);
		public void onError();
	}

	public class «broadcastName.toFirstUpper»BroadcastAdapter implements «listenerInterface» {
		public void onReceive(«broadcast.commaSeperatedTypedOutputParameterList») {
			// empty implementation
		}
		public void onError() {
			// empty implementation
		}
	}

	«IF isSelective(broadcast)»
		public class «filterParameterType» extends BroadcastFilterParameters {
			public «filterParameterType»() {};

			«IF filterParameters.size > 0»
				public «filterParameterType»(«broadcast.commaSeperatedTypedFilterParameterList») {
					«FOR filterParameter : filterParameters»
						super.setFilterParameter("«filterParameter»", «filterParameter»);
					«ENDFOR»
				}
			«ENDIF»
			«FOR filterParameter : filterParameters»
				public void set«filterParameter.toFirstUpper»(String «filterParameter») {
					super.setFilterParameter("«filterParameter»", «filterParameter»);
				}
				public String get«filterParameter.toFirstUpper»() {
					return super.getFilterParameter("«filterParameter»");
				}
			«ENDFOR»
		}

		@JoynrRpcBroadcast(broadcastName = "«broadcastName»")
		abstract String subscribeTo«broadcastName.toFirstUpper»Broadcast(
				«listenerInterface» broadcastListener,
				OnChangeSubscriptionQos subscriptionQos,
				«filterParameterType» filterParameters);

		@JoynrRpcBroadcast(broadcastName = "«broadcastName»")
		abstract String subscribeTo«broadcastName.toFirstUpper»Broadcast(
				«listenerInterface» broadcastListener,
				OnChangeSubscriptionQos subscriptionQos,
				«filterParameterType» filterParameters,
				String subscriptionId);
	«ELSE»
		@JoynrRpcBroadcast(broadcastName = "«broadcastName»")
		abstract String subscribeTo«broadcastName.toFirstUpper»Broadcast(
				«listenerInterface» subscriptionListener,
				OnChangeSubscriptionQos subscriptionQos);

		@JoynrRpcBroadcast(broadcastName = "«broadcastName»")
		abstract String subscribeTo«broadcastName.toFirstUpper»Broadcast(
				«listenerInterface» subscriptionListener,
				OnChangeSubscriptionQos subscriptionQos,
				String subscriptionId);
	«ENDIF»

	abstract void unsubscribeFrom«broadcastName.toFirstUpper»Broadcast(String subscriptionId);
«ENDFOR»
}
'''
	}
}
