// object_mysql_1.c

#include "liblwm2m.h"
#include "lwm2mclient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Inlcude the .h generated by the gen_object tool
#include "object_mysql_15.h"

// Object dimensioning are defined in lwm2mclient.h

/*
 * Multiple instance objects can use userdata to store data that will be shared between the different instances.
 * The lwm2m_object_t object structure - which represent every object of the liblwm2m as seen in the single instance
 * object - contain a chained list called instanceList with the object specific structure mysql_15_instance_t:
 */
typedef struct _mysql_15_instance_
{
    /*
     * The first two are mandatories and represent the pointer to the next instance and the ID of this one. The rest
     * is the instance scope user data (uint8_t test in this case)
     */
    struct    _mysql_15_instance_ * next;    // matches lwm2m_list_t::next
    uint16_t shortID;                                 // matches lwm2m_list_t::id
    // double   resource[ResourceLength_15];
    int      resource[ResourceLength_15];   
   
} mysql_15_instance_t;


static void mysql_15_output_buffer(uint8_t * buffer, int length)
{
    int i;
    uint8_t array[16];

    i = 0;
    while (i < length)
    {
        int j;
        fprintf(stderr, "  ");

        memcpy(array, buffer+i, 16);

        for (j = 0 ; j < 16 && i+j < length; j++)
        {
            fprintf(stderr, "%02X ", array[j]);
        }
        while (j < 16)
        {
            fprintf(stderr, "   ");
            j++;
        }
        fprintf(stderr, "  ");
        for (j = 0 ; j < 16 && i+j < length; j++)
        {
            if (isprint(array[j]))
                fprintf(stderr, "%c ", array[j]);
            else
                fprintf(stderr, ". ");
        }
        fprintf(stderr, "\n");

        i += 16;
    }
}

static uint8_t mysql_15_read(uint16_t instanceId,
                        int * numDataP,
                        lwm2m_data_t ** dataArrayP,
                        lwm2m_object_t * objectP)
{
    mysql_15_instance_t * targetP;
    int  i, j, r;
    char c;
    int  numres = 0;

   
    targetP = (mysql_15_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    // Get the number of actual resources that can be read (not executed)
    numres = 0;
    for (i = 0 ; i < ResourceLength_15 ; i++) {
        if (ro_15.type[i] != 'e') {
           numres++;
        }	   
    }
   
    if (*numDataP == 0) {

        *dataArrayP = lwm2m_data_new(numres);       
        if (*dataArrayP == NULL) {
            // printf("mysql_15_read: returns:  COAP_500_INTERNAL_SERVER_ERROR\n");
           return COAP_500_INTERNAL_SERVER_ERROR;
        }
       
        *numDataP = numres;
              
	// Copy all the resources, except the executables
	i = 0;
        for (j = 0 ; j < ResourceLength_15; j++) {
	    if (ro_15.type[j] != 'e') {
               // printf("copying: %d  %d  type=%c\n",j,ro_15.resources[j],ro_15.type[j]);
               (*dataArrayP)[i].id = ro_15.resources[j];
	       i++;
	    }
        }
       
    }

    // Go through the number of resources (the readable ones)
    for (i = 0 ; i < *numDataP ; i++) {

        if ((*dataArrayP)[i].type == LWM2M_TYPE_MULTIPLE_RESOURCE) {
           return COAP_404_NOT_FOUND;
	}
        // Recources :  15000/0/5700, 15000/0/5701, 15000/0/5702, 15000/0/5703, ...       
        for (j = 0 ; j < ResourceLength_15; j++) {
	   
           r = ro_15.resources[j];
           c = ro_15.type[j];
       
	   // If the resource (e.g. 5700) matches, then...
           if (  (*dataArrayP)[i].id == r ) {
	      
              if (c == 'f') {
                 lwm2m_data_encode_float(atof(ro_15.value[j]), *dataArrayP + i);
              }
	      
              if (c == 'i') {
                 lwm2m_data_encode_int(atoi(ro_15.value[j]), *dataArrayP + i);
              }
       
              if (c == 's') {
                 lwm2m_data_encode_string(ro_15.value[j], *dataArrayP + i);
              }

              if (c == 't') {
                 lwm2m_data_encode_int(time(NULL), *dataArrayP + i);
              }
	      
              if (c == 'o') {       
                 lwm2m_data_encode_string(ro_15.value[j], *dataArrayP + i);
              }
	      
              if (c == 'b') {
		 if (strcmp(ro_15.value[j],"false"))  {
                    lwm2m_data_encode_bool(1, *dataArrayP + i);
		 } else  {
                    lwm2m_data_encode_bool(0, *dataArrayP + i);
	         }
              }
	             
          }
        }
               
    }
   
    return COAP_205_CONTENT;
}

static uint8_t mysql_15_discover(uint16_t instanceId,
                            int * numDataP,
                            lwm2m_data_t ** dataArrayP,
                            lwm2m_object_t * objectP)
{
    int i, j, r;
    bool found;

    // printf("Calling: _discover\n");
   
    // Is the server asking for the full object ?
    if (*numDataP == 0) {
       
       *dataArrayP = lwm2m_data_new(ResourceLength_15);
       
       if (*dataArrayP == NULL) {
          printf("mysql_15_discover: returns:  COAP_500_INTERNAL_SERVER_ERROR\n");       
          return COAP_500_INTERNAL_SERVER_ERROR;
       }
       
       // The number of resources
       *numDataP =  ResourceLength_15;
      
       // Recources :  15000/0/5700, 15000/0/5701, 15000/0/5702, 15000/0/5703, ...
       for (j = 0 ; j < ResourceLength_15 ; j++) {
           (*dataArrayP)[j].id = ro_15.resources[j];
       }
       
    } else {
       
       for (i = 0; i < *numDataP; i++) {

           // Recources :  15000/0/5700, 15000/0/5701, 15000/0/5702, 15000/0/5703, ...       
           found = false; 
           for (j = 0 ; j < ResourceLength_15; j++) {
              r = ro_15.resources[j];
              if (  (*dataArrayP)[i].id == r ) {
                  found = true;
              }
           }
       
           if (found == false) {
              return COAP_404_NOT_FOUND;       
           }
       }
       
    }
 
    if (found == true){	
       return COAP_204_CHANGED;
    }
   
   
    return COAP_205_CONTENT;
}

static uint8_t mysql_15_write(uint16_t instanceId,
                         int numData,
                         lwm2m_data_t * dataArray,
                         lwm2m_object_t * objectP)
{
    mysql_15_instance_t * targetP;
    int i, j, r;
    bool  found;

    targetP = (mysql_15_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    for (i = 0 ; i < numData ; i++) {

        // Recources :  15000/0/5700, 15000/0/5701, 15000/0/5702, 15000/0/5703, ...       
        found = false; 
      
        for (j = 0 ; j < ResourceLength_15; j++) {
           r = ro_15.resources[j];
           if ( dataArray[i].id == r ) {
	      
              // if (1 != lwm2m_data_decode_float(dataArray + i, &(targetP->resource[j]))) {	      
              // if (1 != lwm2m_data_decode_int(dataArray + i, targetP->resource[j])) {
              //   return COAP_400_BAD_REQUEST;
              // }
              found = true;	      
           }
        }
       
        if (found == false) {
           return COAP_404_NOT_FOUND;       
        }

    }

    return COAP_204_CHANGED;
}


uint8_t mysql_15_update(int instance, int resource, char *value) {
   
    lwm2m_object_t * objectP = GLtestobject_15;
    mysql_15_instance_t * targetP;
    int j, r;
    bool found;

    uint16_t instanceId = instance;
    targetP = (mysql_15_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;
   
    found = false; 
     
    for (j = 0 ; j < ResourceLength_15; j++) {
        r = ro_15.resources[j];
        if ( resource == r ) {	   
	   
           //targetP->resource[j] = atof(value);
           targetP->resource[j] = atoi(value);	   
	   strcpy(ro_15.value[j],value);
	   
           found = true;	   
        }
    }
       
    if (found == false) {
       return COAP_404_NOT_FOUND;       
    }
       
    return COAP_204_CHANGED;
}

static uint8_t mysql_15_delete(uint16_t id,
                          lwm2m_object_t * objectP)
{
    mysql_15_instance_t * targetP;

    objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id, (lwm2m_list_t **)&targetP);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    lwm2m_free(targetP);

    return COAP_202_DELETED;
}

static uint8_t mysql_15_create(uint16_t instanceId,
                          int numData,
                          lwm2m_data_t * dataArray,
                          lwm2m_object_t * objectP)
{
    mysql_15_instance_t * targetP;
    uint8_t result;


    targetP = (mysql_15_instance_t *)lwm2m_malloc(sizeof(mysql_15_instance_t));
    if (NULL == targetP) {
       printf("mysql_15_create: returns:  COAP_500_INTERNAL_SERVER_ERROR\n");       
       return COAP_500_INTERNAL_SERVER_ERROR;
    }
   
    memset(targetP, 0, sizeof(mysql_15_instance_t));

    targetP->shortID = instanceId;
    objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, targetP);

    result = mysql_15_write(instanceId, numData, dataArray, objectP);

    if (result != COAP_204_CHANGED)
    {
        (void)mysql_15_delete(instanceId, objectP);
    }
    else
    {
        result = COAP_201_CREATED;
    }

    return result;
}

static uint8_t mysql_15_exec(uint16_t instanceId,
                        uint16_t resourceId,
                        uint8_t * buffer,
                        int length,
                        lwm2m_object_t * objectP)
{

    if (NULL == lwm2m_list_find(objectP->instanceList, instanceId)) return COAP_404_NOT_FOUND;

    switch (resourceId)
    {
    case 1:
        return COAP_405_METHOD_NOT_ALLOWED;
    case 2:
        fprintf(stdout, "\r\n-----------------\r\n"
                        "Execute on %hu/%d/%d\r\n"
                        " Parameter (%d bytes):\r\n",
                        objectP->objID, instanceId, resourceId, length);
        mysql_15_output_buffer((uint8_t*)buffer, length);
        fprintf(stdout, "-----------------\r\n\r\n");
        return COAP_204_CHANGED;
    case 3:
        return COAP_405_METHOD_NOT_ALLOWED;
    default:
        return COAP_404_NOT_FOUND;
    }
}

void mysql_15_display(lwm2m_object_t * object) {
   
    if (  ro_15.active == 0 ) {
       return;
    }
   
#ifdef WITH_LOGS
    fprintf(stdout, "  /%u: Test object, instances:\r\n", object->objID);
    mysql_15_instance_t * instance = (mysql_15_instance_t *)object->instanceList;
    while (instance != NULL)
    {
        fprintf(stdout, "    /%u/%u: shortId: %u, test: %u\r\n",
                object->objID, instance->shortID,
                instance->shortID, instance->test);
        instance = (mysql_15_instance_t *)instance->next;
    }
#endif
}

lwm2m_object_t * mysql_15_get(void) {
   
    // Initialize objects, and resources
    Resource_15_Object_Config();      
   
    lwm2m_object_t * testObj;

    testObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
    int j;
   
    if (NULL != testObj) {
        int i;
        mysql_15_instance_t * targetP;

        memset(testObj, 0, sizeof(lwm2m_object_t));

        testObj->objID = ro_15.object;
       
        int instanceStart = 0;
       
        // Define Instances:  15000/0, 15000/1, 15000/2, 15000/3, ....
    
        for (i=0 ; i < ro_15.instances; i++) {
       
            targetP = (mysql_15_instance_t *)lwm2m_malloc(sizeof(mysql_15_instance_t));
            if (NULL == targetP) return NULL;
           
            memset(targetP, 0, sizeof(mysql_15_instance_t));
            
	    targetP->shortID = instanceStart + i;
	    
	    // Assign the resource a value
            for (j = 0 ; j < ResourceLength_15; j++) {
               targetP->resource[j]  = ro_15.resources[j];
	    }
	          
            testObj->instanceList = LWM2M_LIST_ADD(testObj->instanceList, targetP);
       
        }
       
        /*
         * From a single instance object, two more functions are available.
         * - The first one (createFunc) create a new instance and filled it with the provided informations. If an ID is
         *   provided a check is done for verifying his disponibility, or a new one is generated.
         * - The other one (deleteFunc) delete an instance by removing it from the instance list (and freeing the memory
         *   allocated to it)
         */
       
        testObj->readFunc     = mysql_15_read;
        testObj->discoverFunc = mysql_15_discover;
        testObj->writeFunc    = mysql_15_write;
        testObj->executeFunc  = mysql_15_exec;
        testObj->createFunc   = mysql_15_create;
        testObj->deleteFunc   = mysql_15_delete;
    }

    return testObj;
}

void mysql_15_free(lwm2m_object_t * object)
{
    LWM2M_LIST_FREE(object->instanceList);
    if (object->userData != NULL)
    {
        lwm2m_free(object->userData);
        object->userData = NULL;
    }
    lwm2m_free(object);
}

