lib_LTLIBRARIES = libmozapoc.la

libdir=@COMPONENT_DIR@

AM_LDFLAGS=-R @XPCOM_LIBDIR@

libmozapoc_la_SOURCES = \
        mozApocModule.cpp \
        mozApocPrefService.cpp \
        mozApocPrefBranch.cpp \
        mozApocPolicyData.cpp \
        mozApocPolicyModel.cpp \
        mozApocPolicyModelBuilder.cpp \
        mozApocPolicyModelFromDOM.cpp \
        mozApocPolicyNodeData.cpp \
        mozApocPolicyNodeDOMData.cpp \
        mozApocPolicyModelFromXML.cpp \
        mozApocPolicyComponentData.cpp \
        mozApocPolicyComponentDOMData.cpp \
        mozApocSaxWrapper.cpp \
        mozApocPolicyBackend.cpp \
        mozApocNotificationImpl.cpp \
        mozApocPathParser.cpp \
        mozApocStringList.cpp \
        mozApocPAPI.cpp

install-local-data:
	echo $(AM_LDFLAGS)
