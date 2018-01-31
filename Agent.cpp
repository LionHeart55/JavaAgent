/*
 * Agent.cpp
 *
 *  Created on: Jan 26, 2018
 *      Author: shaharshocron
 */

//
//  Created by Rodrigo Agustin Peinado on 5/7/16.
//  Copyright © 2016 Rodrigo Agustin Peinado. All rights reserved.
//
#include "Agent.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <jvmti.h>
#include "stdlib.h"

using namespace std;

#define countof(array) (sizeof(array)/sizeof(array[0]))

using namespace std;

bool setsecuritymanager_checked=false;

// ------
static const char* DUMP_DIRECTORY = "/tmp/";

//static jrawMonitorID* mutexFlag;

int main() { return 0; }

static void check(jvmtiEnv *jvmti, jvmtiError errnum, const char *str)
{
    if ( errnum != JVMTI_ERROR_NONE ) {
        char *errnum_str = NULL;
        jvmti->GetErrorName(errnum, &errnum_str);

        cout << "ERROR: JVMTI";
    }
}

/* -------------------------------------------------------------------

static void enter_critical_section(jvmtiEnv *jvmti)
{
    jvmtiError error = jvmti->RawMonitorEnter(mutex);
    check(jvmti, error, "Cannot enter with raw monitor");
}


static void exit_critical_section(jvmtiEnv *jvmti)
{
    jvmtiError error = jvmti->RawMonitorExit(mutex);
    check(jvmti, error, "Cannot exit with raw monitor");
}
 --------------------------------------------------------------------*/

static void printObject(jvmtiEnv *jvmti_env, jthread thread, jvmtiLocalVariableEntry* entry) {
    jvmtiError error;
    jint int_value_ptr;
    jlong long_value_ptr;
    jfloat float_value_ptr;
    jdouble double_value_ptr;
    jobject object_value_ptr;
    jint depht = 0;

    //cout << entry->signature << ":" << entry->name << endl;

    switch (entry->signature[0]) {
        case 'Z' :
        case 'B' :
        case 'S' :
        case 'C' :
        case 'I' :
            error = jvmti_env->GetLocalInt(thread, depht, entry->slot, &int_value_ptr);
            if(error == JVMTI_ERROR_NONE) {
                cout << "Type: Integer, Name: " << entry->name << ", Value: " << int_value_ptr << endl;
            } else {
                cout << "error: " << error << endl;
            }
            break;
        case 'J' :
            error = jvmti_env->GetLocalLong(thread, depht, entry->slot, &long_value_ptr);
            if(error == JVMTI_ERROR_NONE) {
                cout << "Type: Long, Name: " << entry->name << ", Value: " << long_value_ptr << endl;
            } else {
                cout << "error: " << error << endl;
            }
            break;
        case 'F' :
            error = jvmti_env->GetLocalFloat(thread, depht, entry->slot, &float_value_ptr);
            if(error == JVMTI_ERROR_NONE) {
                cout << "Type: Float, Name: " << entry->name << ", Value: " << float_value_ptr << endl;
            } else {
                cout << "error: " << error << endl;
            }
            break;
        case 'D' :
            error = jvmti_env->GetLocalDouble(thread, depht, entry->slot, &double_value_ptr);
            if(error == JVMTI_ERROR_NONE) {
                cout << "Type: Double, Name: " << entry->name << ", Value: " << double_value_ptr << endl;
            } else {
                cout << "error: " << error << endl;
            }
            break;
        case 'L' :
            error = jvmti_env->GetLocalObject(thread, depht, entry->slot, &object_value_ptr);
            if(error == JVMTI_ERROR_NONE) {
                // get object attributes

                cout << "Type: LocalObject, Name: " << entry->name << ", Length: " << entry->length
                		 << ", Value: " << object_value_ptr << endl;


            } else {
                cout << "error: " << error << endl;
            }
            break;
    }
}

static const char* getReferenceType(jint type) {
	switch (type) {
	        case 0:
	        		return "Invalid Reference";
	        case 1:
	        		return "Local Reference";
	        case 2:
	        		return "Global Reference";
	        case 3:
	        		return "Weak Global reference";
	        default:
	        		return "Undefined:";
	}
}

static int getLineNumberForMethod(jvmtiEnv *jvmti_env, jvmtiFrameInfo methodFrame)
{
	jvmtiError error;
	jvmtiLineNumberEntry* table = NULL;
	jint entry_count = 0;
	int lineNumberCount;
	int lineNumber = -1;
	jlocation prevLocationId = -1;
	if (methodFrame.location == -1)
		return lineNumber;
	error = jvmti_env->GetLineNumberTable(methodFrame.method,
			&entry_count, &table);

	check(jvmti_env, error, "GetLineNumberTable: ");
	if (error == JVMTI_ERROR_NONE)
	{
		prevLocationId = table[0].start_location;
		for (lineNumberCount = 1; lineNumberCount < entry_count; lineNumberCount++)
		{
			if ((table[lineNumberCount].start_location > methodFrame.location)
					&& (methodFrame.location >= prevLocationId))
			{
				lineNumber = table[lineNumberCount - 1].line_number;
				break;
			}
			prevLocationId = table[lineNumberCount].start_location;
		}
		if (lineNumberCount == entry_count)
		{
			if (entry_count == 1)
				lineNumber = table[lineNumberCount - 1].line_number;
			else
				lineNumber = -1;
		}
	}
	return lineNumber;
}

static char * getExceptionMessage(JNIEnv* env, jobject exception)
{
	jmethodID getMessageMethodId;
	jstring jExceptionMessage;
	int msgLength;
	char * msgArray = NULL;

	jclass clazz = env->FindClass("java/lang/Throwable");
	getMessageMethodId = env->GetMethodID(clazz, "getMessage", "()Ljava/lang/String;");
	if (getMessageMethodId != NULL)
	{
		jExceptionMessage = (jstring) env->CallObjectMethod(exception, getMessageMethodId);
		if (jExceptionMessage != NULL)
		{
			msgLength = env->GetStringLength(jExceptionMessage);
			if (msgLength > 0)
			{
				msgArray = (char *) malloc(msgLength + 1); // To be freed explicitly by the caller of this function
				if (msgArray == NULL)
					return NULL;
				env->GetStringUTFRegion(jExceptionMessage, 0, msgLength, msgArray);
				msgArray[msgLength] = '\0';
			}
		}
	}
	return msgArray;
}

static void JNICALL Exception(jvmtiEnv *jvmti_env,JNIEnv* jni_env,jthread thread,jmethodID method,jlocation location,jobject exception,jmethodID catch_method,jlocation catch_location)
{
    jint type;
    jint entryCount;
    jvmtiError error;
    jvmtiLocalVariableEntry* localVariableEntry;
    localVariableEntry = NULL;
    char * exceptionMessage = NULL;

    /*klass=jni_env->GetObjectClass(exception);
	  jvmti_env->GetMethodName(method,&method_name,&method_signature,&generic_ptr_method);

	  cout << "Method: " << method_name << ", Signature: " << method_signature << endl;*/

    type= jni_env->GetObjectRefType(exception);

    cout << "Exception Reference Type: " << getReferenceType(type) << endl;

    if(type>0)
    {
    		exceptionMessage = getExceptionMessage(jni_env, exception);
    		cout << "Exception Message: " << exceptionMessage << endl;

    		//----
    	    jint num_frames=0;
    		error = jvmti_env->GetFrameCount(thread, &num_frames);
    		if (error != JVMTI_ERROR_NONE) {
    			cout << "Could not get the frame count." << endl;
    			return;
    		}

    		jvmtiFrameInfo frames[num_frames];
    		if(num_frames > 0)
    		{
    			jint count;
    			error = jvmti_env->GetStackTrace(NULL, 0, 10, (jvmtiFrameInfo*)&frames, &count);
    			cout << "Stacktrace: Frame count=" << count << " num of frames: " << num_frames << endl << "------------------------------------------------" << endl;

    			if (error != JVMTI_ERROR_NONE) {
    			     printf("(GetThreadInfo) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, error);
    			     //describe(err);
    			     printf("\n");
    			}
    			else if (error == JVMTI_ERROR_NONE && count >=1) {
    				printf("Number of records filled: %d\n", count);
    			     char *methodName;
    			     //methodName = "yet_to_call()";
    			     char *declaringClassName;
    			     jclass declaring_class;

    			     printf("Exception Stack Trace\n");
    			     printf("=====================\n");
    			     printf("Stack Trace Depth: %d\n", count);

    			     for (int i=0; i < count; i++) {
    			             error = jvmti_env->GetMethodName(frames[i].method, &methodName, NULL, NULL);
						  if (error == JVMTI_ERROR_NONE) {
							  error = jvmti_env->GetMethodDeclaringClass(frames[i].method, &declaring_class);
							  error = jvmti_env->GetClassSignature(declaring_class, &declaringClassName, NULL);
							  int lineNum = getLineNumberForMethod(jvmti_env, frames[i]);
							  if (error == JVMTI_ERROR_NONE) {
									printf("at method %s( %d ) in class %s\n", methodName, lineNum, declaringClassName);
							  }
    			             }

						  error = jvmti_env->GetLocalVariableTable(frames[0].method, &entryCount, &localVariableEntry);

						  if(error == JVMTI_ERROR_NONE) {

							  jvmtiLocalVariableEntry* entry = localVariableEntry;
							  cout << "Variables: " << entryCount << endl;

							  for(int i = 0; i < entryCount; i++, entry++) {
									// print object's details
								  printObject(jvmti_env, thread, entry);

								  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(entry->signature));
								  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(entry->name));
								  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(entry->generic_signature));
							  }
						      jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(localVariableEntry));
						  } else if(error == JVMTI_ERROR_ABSENT_INFORMATION) {
					            cout << "<NO LOCAL VARIABLE INFORMATION AVAILABLE>" << endl;
					      } else {
					            cout << "<ERROR>" << endl;
					      }
    			     }
    			}
    		}
    }
    if(exceptionMessage != NULL)
    		free(exceptionMessage);
}

//-------
static void JNICALL loadClass(jvmtiEnv *jvmti_env,JNIEnv* jni_env,jthread thread,jclass klass)
{
    char *class_name;
    char *generic_ptr_class;

    jvmti_env->GetClassSignature(klass, &class_name,&generic_ptr_class);

        cout << "loadClass: Agent_Registered" << endl;
        jvmti_env->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION,(jthread)NULL);
        jvmti_env->SetEventNotificationMode(JVMTI_DISABLE,JVMTI_EVENT_CLASS_PREPARE,(jthread)NULL);

    jvmti_env->Deallocate((unsigned char *)class_name);
    jvmti_env->Deallocate((unsigned char *)generic_ptr_class);
}

static bool startsWith(const char* str, const char* subStr) {
	// look for subStr at the start of str
	if ( strncmp(subStr, str, strlen(subStr)) == 0 )
	    return true;
	return false;
}

const char* clazzes[] = { "java", "sun", "scala" };

static bool isUserClass(const char* clz) {
	for(int i=0; i<3; i++)
		if(	startsWith(clz, clazzes[i]) )
				return false;
	return true;
}

static void JNICALL showClass(jvmtiEnv *jvmti,
    JNIEnv* env,
    jclass class_being_redefined,
    jobject loader,
    const char* name,
    jobject protection_domain,
    jint class_data_len,
    const unsigned char* class_data,
    jint* new_class_data_len,
    unsigned char** new_class_data)
{
    //enter_critical_section(jvmti); {
        if(isUserClass(name))
        {
        		cout << "Loading Class: " << name << "\n";
        		std::string str(name);
			for(int i=0; i<str.length(); i++)
			{
				if(str[i] == '/')
					str[i] = '.';
			}

			char file[512] = "";
			sprintf(file, "%s%s.class", DUMP_DIRECTORY, str.c_str());

			ofstream myfile;
			myfile.open (file);
			for(int i=0; i<class_data_len; i++)
				myfile << class_data[i];
			myfile.close();
        }
   // } exit_critical_section(jvmti);
}


//---------------
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
    static jvmtiEnv *jvmti=NULL;
    static jvmtiCapabilities capabilities;
    jvmtiEventCallbacks callbacks;
    jint res;


    cout << "Agent_OnLoad" << endl;

    res = vm->GetEnv((void **)&jvmti, JVMTI_VERSION_1);
    if (res != JNI_OK||jvmti==NULL)
    {
        cout << "ERROR: Unable to access JVMTI Version 1" << endl;
    }


    (void)memset(&capabilities,0, sizeof(capabilities));

    capabilities.can_generate_exception_events=1;
    capabilities.can_get_line_numbers=1;
    capabilities.can_access_local_variables=1;
    capabilities.can_tag_objects=1;
    //---
    capabilities.can_generate_all_class_hook_events  = 1;

    jvmtiError error = jvmti->AddCapabilities(&capabilities);

    check(jvmti,error,"Unable to get necessary capabilities.");

    (void)memset(&callbacks,0, sizeof(callbacks));

    //---
    //callbacks.ClassPrepare = &loadClass;
    callbacks.ClassFileLoadHook = &showClass;
    callbacks.Exception=&Exception;

    jvmti->SetEventCallbacks(&callbacks, (jint)sizeof(callbacks));

    //---
    jvmti->SetEventNotificationMode(JVMTI_ENABLE,JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, (jthread)NULL);
    //jvmti->SetEventNotificationMode(JVMTI_ENABLE,JVMTI_EVENT_CLASS_PREPARE,(jthread)NULL);

    jvmti->SetEventNotificationMode(JVMTI_ENABLE,JVMTI_EVENT_EXCEPTION,(jthread)NULL);

    //jvmti->CreateRawMonitor("agent data", &mutex);
    //    check(jvmti, error, "Cannot create raw monitor");

    return JNI_OK;
}


JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
    cout << "Agent_OnUnload" << endl;
}


