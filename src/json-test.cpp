#include <iostream>

extern "C" {
    #include "jsonc/json.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

using namespace std;

static void wf(const char *file_name, const char *buf, size_t len)
{
    FILE *fp = fopen(file_name, "w");
    if(fp == NULL)
        return;
    
    fwrite(buf, 1, len, fp);
    fclose(fp);
}

static char *rf(const char *fname)
{
    struct stat st;
    char *buf;
    if(stat(fname, &st) != 0)
        return NULL;
    buf = (char *)malloc(st.st_size+1);
    buf[st.st_size] = 0;
    FILE *fp = fopen(fname, "r");
    fread(buf, 1, st.st_size, fp);
    fclose(fp);
    return buf;
}

static int splitc(char *string, char *fields[], int  nfields, char sep )
{
    char *p, *p1;
    int i;

    p = string;
    fields[0] = p;
    i = 1;
    while( (p1=strchr(p,sep))!= NULL && i<nfields){
        p1[0] = 0; p1++;
        fields[i] = p1;
        p = p1; i++;
    }
    return i;
}

static void __json_print(json_object *pj, const int indent)
{
    char indent_str[indent+1];
    memset(indent_str, ' ', indent); indent_str[indent] = 0;
    
	json_object_object_foreach(pj, key, val){
        cout << indent_str << '[' << key << "] = ";
		switch(json_object_get_type(val))
		{
		case json_type_null:
			cout << "<NULL>";
			break;
		case json_type_boolean:
			cout << (int)json_object_get_boolean(val);
			break;
		case json_type_double:
			cout << json_object_get_double(val);
			break;
		case json_type_int:
			cout << (long)json_object_get_int64(val);
			break;
		case json_type_string:
			cout <<  json_object_get_string(val);
			break;
		case json_type_object:
		{
            cout << endl;
			__json_print(val, (const int)(indent+2));
			break;
		}
		case json_type_array:
		{
            cout << "[" << endl;
			int arrlen = json_object_array_length(val);
			for(int i=0; i<arrlen; i++)
			{
				json_object *pj1 = json_object_array_get_idx(val, i);

				switch(json_object_get_type(pj1))
				{
				case json_type_object:
				case json_type_array:
					{ 
					__json_print(pj1, (const int)(indent+2));
					break;
					}
				case json_type_null:
					cout << "- <NULL>";
					break;
				case json_type_boolean:
					cout << "- " << (int)json_object_get_boolean(pj1);
					break;
				case json_type_double:
					cout << "- " << json_object_get_double(pj1);
					break;
				case json_type_int:
					cout << "- " <<  (long)json_object_get_int64(pj1);
					break;
				case json_type_string:
					cout << "- " << json_object_get_string(pj1);
					break;
				}
			}
			break;
		}
		}
        cout << endl;
	}
}

void print_json(const char *file_name) {
    char *str = rf(file_name);
    if(str == NULL)
        return;
    json_object *jo = NULL;
    json_type tp;

    enum json_tokener_error jerr;
    jo = json_tokener_parse_verbose(str, &jerr);
    free(str);
    if(is_error(jo)){
        printf("==JSONERR: json_tokener_parse failed， errcode: %d, err: %s\n",
                jerr, json_tokener_error_desc(jerr ));
        return;
    }
    tp = json_object_get_type(jo);
    if(tp != json_type_object){
        printf("==JSONERR: not start with a Object\n");
        return;
    }


    json_object *jroot = NULL;
    jroot = json_object_object_get(jo,"ROOT");
    if(jroot != NULL && json_object_get_type(jroot) != json_type_object)
    {
        printf("==JSONERR: not start with ROOT\n");
        return;
    }
    if(jroot == NULL){ // compatible with previous none-header input
        jroot = jo;
    }

    __json_print(jroot, 2);

    json_object_put(jo);
}

static const char * INC_KEY(char *k, int i){
    sprintf(k, "key-%d", i);
    return k;
}
static void test_new_json(const char *file_name) {
    json_object *pj;

    char key[32];
    int i = 0;
	json_object *pj1, *pj2;

	pj = json_object_new_object();

    json_object_object_add(pj, INC_KEY(key, i++), json_object_new_object());
    json_object_object_add(pj, INC_KEY(key, i++), json_object_new_string("hello"));
    json_object_object_add(pj, INC_KEY(key, i++), json_object_new_int64(1238));
    json_object_object_add(pj, INC_KEY(key, i++), json_object_new_double(982.9));
    json_object_object_add(pj, INC_KEY(key, i++), NULL);
    pj1 = json_object_new_object();
    json_object_object_add(pj1, INC_KEY(key, i++), json_object_new_string("hello1"));
    json_object_object_add(pj, INC_KEY(key, i++), pj1);

    pj2 = json_object_new_array();
    json_object_array_add(pj2, json_object_new_string("hello2"));
    json_object_array_add(pj2, json_object_new_string("hello3"));
    json_object_array_add(pj2, json_object_new_string("hello4"));
    json_object_object_add(pj, INC_KEY(key, i++), pj2);

    const char *sret = json_object_to_json_string(pj);
    cout << sret << endl;

    wf(file_name, sret, strlen(sret));

    json_object_put(pj);

}


    #include <hiredis/hiredis.h>
    #include "comm.h"


void redis_test(){

    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err) {
        if (c) {
            printf("Error: %s\n", c->errstr);
            // handle error
        } else {
            printf("Can't allocate redis context\n");
        }
    }

    redisReply *reply;
    reply = (redisReply *)redisCommand(c, "SET foo %s", "bar");
    cout << "set foo: " << reply->str << endl;
    freeReplyObject(reply);
    reply = (redisReply *)redisCommand(c, "GET foo");
    cout << "get foo: ---- " << reply->str << endl;
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(c, "SET foo1 %s", "bar");
    cout << "set foo1: " << reply->str << endl;
    freeReplyObject(reply);


    char addr[32];
    tcp_get_sockaddr(c->fd, addr);
    cout << "addr: " << addr << endl;

    redisFree(c);
}

#include <yaml-cpp/yaml.h>
void test_yaml() {
    YAML::Node conf = YAML::LoadFile("../conf/acc1.yaml");

    if (conf["id"]) {
        
    }

    const std::string id = conf["id"].as<std::string>();
    const std::string desc = conf["desc"].as<std::string>();
    //login(username, password);
    cout << id << " " << desc << endl;
    cout << conf["port"].as<int>() << endl;

    cout << "----- " << conf["redis_addr"].as<string>() << endl;

    // std::ofstream fout("config.yaml");
    // fout << config;

    // https://stackoverflow.com/questions/29012599/parsing-json-yaml-array-with-yaml-cpp
    YAML::Node config = YAML::LoadFile("../conf/lcc1.yaml");
    for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
        std::string key = it->first.as<std::string>();
        YAML::Node value = it->second;
        // here, you can check what type the value is (e.g., scalar, sequence, etc.)
        switch (value.Type()) {
            case YAML::NodeType::Scalar: // do stuff
                break;
            case YAML::NodeType::Sequence: // do stuff
                break;
            // etc.
        }
    }
}

int main(int argc, char *argv[])
{
    // print_json("../conf/a.json");

    // test_new_json("../conf/b.json");

    redis_test();

    test_yaml();

    return 0;
}
