package io.joynr.generator.cpp.proxy
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
import io.joynr.generator.cpp.util.CppInterfaceUtil
import io.joynr.generator.cpp.util.CppStdTypeUtil
import io.joynr.generator.cpp.util.JoynrCppGeneratorExtensions
import io.joynr.generator.cpp.util.TemplateBase
import io.joynr.generator.templates.InterfaceTemplate
import io.joynr.generator.templates.util.NamingUtil

class InterfaceFireAndForgetProxyHTemplate extends InterfaceTemplate {
	@Inject extension JoynrCppGeneratorExtensions
	@Inject extension TemplateBase
	@Inject extension CppStdTypeUtil

	@Inject extension CppInterfaceUtil
	@Inject private extension NamingUtil

	override generate()
'''
«val interfaceName =  francaIntf.joynrName»
«val className = interfaceName + "Proxy"»
«val fireAndForgetClassName = interfaceName + "FireAndForgetProxy"»
«val headerGuard = ("GENERATED_INTERFACE_"+getPackagePathWithJoynrPrefix(francaIntf, "_")+
	"_"+interfaceName+"FireAndForgetProxy_h").toUpperCase»
«warning()»

#ifndef «headerGuard»
#define «headerGuard»

#include "joynr/PrivateCopyAssign.h"
«getDllExportIncludeStatement()»
#include "«getPackagePathWithJoynrPrefix(francaIntf, "/")»/«className»Base.h"

«FOR parameterType: getRequiredIncludesFor(francaIntf).addElements(includeForString)»
	#include «parameterType»
«ENDFOR»

#include <memory>

«getNamespaceStarter(francaIntf)»
/**
 * @brief FireAndForget proxy for interface «interfaceName»
 *
 * @version «majorVersion».«minorVersion»
 */
class «getDllExportMacro()» «fireAndForgetClassName»: virtual public I«interfaceName»FireAndForget, virtual public «interfaceName»ProxyBase {
public:
	/**
	 * @brief Parameterized constructor
	 * @param messagingAddress The address
	 * @param connectorFactory The connector factory
	 * @param cache The client cache
	 * @param domain The provider domain
	 * @param qosSettings The quality of service settings
	 * @param cached True, if cached, false otherwise
	 */
	«fireAndForgetClassName»(
			std::shared_ptr<const joynr::system::RoutingTypes::Address> messagingAddress,
			joynr::ConnectorFactory* connectorFactory,
			joynr::IClientCache* cache,
			const std::string& domain,
			const joynr::MessagingQos& qosSettings,
			bool cached
	);


	«produceFireAndForgetMethodDeclarations(francaIntf, false)»

	friend class «className»;

private:
	DISALLOW_COPY_AND_ASSIGN(«fireAndForgetClassName»);
};
«getNamespaceEnder(francaIntf)»
#endif // «headerGuard»
'''
}