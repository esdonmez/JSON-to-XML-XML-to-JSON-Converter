/*	This code can parse a JSON file and convert it to a XML part and vice versa. But during converting the XML file to a JSON
 *	file, it can only convert attributes and strings, digits, and other characters. When i tried to add arrays, i faced with lots 
 *	of problems and i left its part in the code blank. Firstly, i tried to create list structs, but after some malloc errors and
 *  realizing using struct is not necessary, i changed all the structure of the assignment. I wrote a function that finds arrays
 *	in a XML file but i couldn't fill its inside properly because of json objects so i left the inside of it blank. 
 */

/*	
 *	Elif Seray Dönmez
 *	2014510112
 */


#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <json/json.h>
#include <ctype.h>

void json_parse(json_object * jobj, xmlNode *rootnode);
void json_array_parse(json_object * jobj, char * key, xmlNode *rootnode);
void xml_add_string(json_object * jobj, xmlNode * root_element, const xmlChar *key, const xmlChar *object);
void xml_parse(json_object * jobj, xmlNode * root_element);
int isArray(xmlNode *cur_node);

int i = 0;
json_object *jobj2;
json_object *jsonObject;
xmlNode * root;
int counter = 0;

void main(int argc, char *argv[]) 
{
	if(strstr(argv[1], ".json") != NULL)   /// Reading json document and parse to xml
	{
		xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
		jsonObject = json_object_from_file(argv[1]);
 		xmlNodePtr root_node = xmlNewNode(NULL, "root");
		xmlDocSetRootElement(doc, root_node);

		json_parse(jsonObject, root_node); 		///  Json parse
		xmlSaveFormatFileEnc("XML.xml", doc, "UTF-8", 1);   ///  Final xml file

		xmlFreeDoc(doc);
		xmlCleanupParser();
		xmlMemoryDump();
	}

	else if(strstr(argv[1], ".xml") != NULL)   ///  Reading xml document and parse to json
	{
		i = 0;
		xmlDoc * doc = xmlReadFile(argv[1], NULL, 0);
		jsonObject = json_object_new_object();
		
		xmlNode * root_element = xmlDocGetRootElement(doc);   ///  Get the root element
		root = root_element;								  ///  The following part is for root implementing the root element

		while (root->type != 1)		///  Until it finds an element_node, it searches nodes
		{
			root = root->next;		 
		}

		if(root->children->next != NULL || root->properties != NULL)   ///  If the root is object
    	{
    		xmlAttr* attribute = NULL;
    		jobj2 = json_object_new_object();
    		json_object_object_add(jsonObject, root->name, jobj2);
    		
    		for(attribute = root->properties; attribute; attribute = attribute->next)  /// Searches for the attributes of the root element
			{
				if(attribute->type == 2)   ///  If attribute node
					json_object_object_add(jobj2,attribute->name ,json_object_new_string((char*)attribute->children->content));
			}
    	}	

    	else   ///  If not object 
    	{
    		json_object_object_add(jsonObject,root->name ,json_object_new_string((char*)root->children->content));
    	}

		xml_parse(jsonObject, root_element->children);
    	json_object_to_file("JSON.json", jsonObject);     ///  Final json file
    	xmlFreeDoc(doc);
	}	
	xmlCleanupParser();
}

void xml_parse(json_object * jobj, xmlNode * root_element)
{
  	xmlNode *cur_node = NULL;

  	for (cur_node = root_element; cur_node; cur_node = cur_node->next)
  	{
    	if (cur_node->type == 1)  ///  If element node
    	{
    		if(cur_node->children->next != NULL || cur_node->properties != NULL) ///  If the node is object
    		{
    			if(isArray(cur_node) == 1) /// Checking if it is array or not. If array then it returns 1
				{
					if(cur_node->properties != NULL || cur_node->children->next != NULL)
					{
						printf("%s\n", cur_node->name);
					}
				}

    			counter = xmlChildElementCount(cur_node);    ///  Child counter
    			json_object_object_add(jobj, cur_node->name, json_object_new_object());
    		}	

    		else   ///  If not object
    		{
    			json_object_object_add(jobj, cur_node->name, json_object_new_string((char*)cur_node->children->content));
    		}

    		if(cur_node->properties != NULL)  ///  If there is an attribute
			{
				xmlAttr *attribute = NULL;

				for(attribute = cur_node->properties; attribute; attribute = attribute->next)   /// Search attributes
				{
					if(attribute->type == 2)  ///  If attribute node
					{
						json_object_object_add(jobj2,attribute->name ,json_object_new_string(attribute->children->content));
					}
				}
			}
    	}

   		else if(cur_node->type == 2)   ///  If element node
    	{
    		if(cur_node->next->children != NULL && cur_node->next != NULL)
    		{
    			if(isArray(cur_node) == 1)  ///  Control if it is array or not. If it returns 1 then it is array
				{
					printf("%s\n", cur_node->name);
				}
    			json_object_object_add(jobj, cur_node->next->name, json_object_new_string((char*)cur_node->next->children->content));
    		}
    	}	
    	i = 0;
    	xml_parse(jobj, cur_node->children);
  	}
}

int isArray(xmlNode *cur_node)   ///  Control if there is an array or not
{
	xmlNode *temp = NULL;
	int i = 0;

	for(i = 0; i < counter; i++)   ///   Travels through its children
	{
		if(strcmp(cur_node->children->next->name, cur_node->children->next->next->next->name) == 0)
		{
		 	return 1;	
		}
	}
	return 0;
}

void json_parse(json_object * jobj, xmlNode *rootnode)  ///  Json to XML parse
{
	enum json_type type;
	
	json_object_object_foreach(jobj, key, val) 
	{
		type = json_object_get_type(val);

		if(type == json_type_object)   ///  If the type is object
		{
			xmlNode* node = xmlNewChild(rootnode, NULL, BAD_CAST key, NULL);
			json_parse(json_object_object_get(jobj,key), node);
		}

		else if(type == json_type_array)   ///  If the type is array
		{
			json_array_parse(jobj, key, rootnode);
		}

		else   ///  If the type is string, integer, double etc.
		{
			xmlNewChild(rootnode,NULL, BAD_CAST key, BAD_CAST json_object_get_string(val));
		}
	}
}

void json_array_parse(json_object * jobj, char * key, xmlNode *rootnode)   ///  It the type is array, then this function works
{
	json_object * jobject = json_object_object_get(jobj, key);

	int i;
	for(i = 0; i < json_object_array_length(jobject); i++)   ////  Travels until it reaches the length of jobject
	{
		json_object * json_id = json_object_array_get_idx(jobject, i);

		if(json_object_get_type(json_id) == json_type_object)   ///   If the type is object
		{
			xmlNode* node = xmlNewChild(rootnode, NULL, BAD_CAST key, NULL);
			json_parse(json_id, node);
		}

		else if(json_object_get_type(json_id) == json_type_array)   ///  If the type is array
		{
			json_array_parse(json_id, NULL, rootnode);
		}

		else   ///  If the type is string, integer, double etc.
		{
			xmlNewChild(rootnode,NULL, BAD_CAST key, BAD_CAST json_object_get_string(json_id));
		}	
	}	
}