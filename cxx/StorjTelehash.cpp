/*
 * Copyright (c) 2015, Shinya Yagyu
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its 
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <string> 

#include "StorjTelehash.hpp"

int StorjTelehash::status=0;
int StorjTelehash::count=0;
link_t StorjTelehash::targetLink=NULL;
char StorjTelehash::globalIP[3*4+3+1]; ;

/**
 * links needed to broadcast.
 */
static list<link_t> broadcastee;
/**
 * registered factory instance that creates ChannelHander instance.
 * Construct On First Us
 */
map<string,ChannelHandlerFactory *> &factories(){
    static map<string,ChannelHandlerFactory *> _factories;
    return _factories;
}

/**
 * handler when receving a broadcast.
 * Construct On First Us
 */
map<string,ChannelHandler *>& broadcastHandlers(){
    static map<string,ChannelHandler *> _broadcastHandlers;
    return _broadcastHandlers;
}

int StorjTelehash::isLocal(char *adr){
    if(!strcmp(adr,"127.0.0.1") || !strncmp(adr,"10.",3) 
        || !strncmp(adr,"192.168.",7)) return 1;
    if(!strncmp(adr,"172.",4) && adr[6]=='.'){
        int tmp=(adr[4]-'0')*10+(adr[5]-'0');
        if(16<=tmp && tmp<=31){
            return 1;
        }
    }
    return 0;
}

char *StorjTelehash::getGlobalIP(char *ip){
    struct ifaddrs *ifap = NULL, *ifa = NULL;
    struct sockaddr_in *sa = NULL;
    char *addr = NULL;

    ip[0]='\0';
    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            if(!isLocal(addr)){
                LOG("found global IP address %s", addr);
                strcpy(ip,addr);
            }
        }
    }
    freeifaddrs(ifap);
    return ip;
}

string &StorjTelehash::getMyId(string &id){
    id = mesh->id->hashname;
    return id;
}

/*
 * location={"hashname":"yd2g2jast2bmlvr76xukaiyuh555vmxug7d7ocwvjo54kl3ebfga",
 * "keys":{"1a":"al45izsjxe2sikv7mc6jpnwywybbkqvsou"},
 *         "paths":[{"type":"udp4","ip":"127.0.0.1","port":1234}]}
*/
string &StorjTelehash::getMyLocation(string &location){
	char ip[3*4+3+1];
    size_t len = 3; // []\0
    char *paths;
    lob_t json, path;

    getGlobalIP(ip);
    if(ip[0]!='\0'){
        strcpy(globalIP,ip);
    }
    json = lob_new();
    lob_set(json,(char*)"hashname",mesh->id->hashname);
    lob_set_raw(json,(char*)"keys",0,
                (char*)mesh->keys->head,mesh->keys->head_len);

    paths = (char *)malloc(len);
    sprintf(paths,"[");
    for(path = mesh->paths;path;path = lob_next(path)){
        lob_t path_=lob_copy(path);
        if(globalIP[0]!='\0' && !lob_get_cmp(path_,(char*)"ip",
                                             (char*)"127.0.0.1")){
            lob_set(path_,(char*)"ip",globalIP);
        }
        len += path_->head_len+1;
        paths = (char *)realloc(paths, len);
        sprintf(paths+strlen(paths),"%.*s,",(int)path_->head_len,path_->head);
        lob_free(path_);
    }
    sprintf(paths+(strlen(paths)-1),"]");
    lob_set_raw(json,(char *)"paths",0,paths,strlen(paths));
    char *jsonStr=lob_json(json);
    location = jsonStr;
    free(paths);
    lob_free(json);
    return location;
}

int StorjTelehash::writeID(){
    lob_t options = lob_new();
    e3x_init(options);

    LOG("*** generating keys ***");
    lob_t secrets = e3x_generate();
    lob_t keys = lob_linked(secrets);
    
    hashname_t id = hashname_keys(keys);
    lob_t json = lob_new();
    lob_set(json,(char *)"hashname",id->hashname);
    lob_set_raw(json,(char *)"keys",0,(char*)keys->head,keys->head_len);
    lob_set_raw(json,(char *)"secrets",0,(char*)secrets->head,
                secrets->head_len);
  
    FILE *fdout = fopen("id.json","wb");
    fwrite(json->head,1,json->head_len,fdout);
    fclose(fdout);
    lob_free(options);
    lob_free(json);
    lob_free(secrets);
    return 0;
}

void StorjTelehash::onLink(link_t link){
    if(link==targetLink && link_up(link)){
        LOG("linked up");
        status=1;
    }
}

void StorjTelehash::sendOnChannel(link_t link, e3x_channel_t chan,
                                      lob_t packet,ChannelHandler *ch){
    if(!lob_get_cmp(packet,(char *)"end",(char *)"true")) {
        LOG("pointer delteted=%p",ch);
        delete(ch);

        return;
    }
    lob_t data=lob_get_json(packet,(char *)"data");
    char *json=ch->handle((char *)lob_json(data));
    lob_t tmp = lob_new();
    lob_set_uint(tmp,(char *)"c",e3x_channel_id(chan));
    if(json){
        lob_set_raw(tmp,(char *)"data",4,json,strlen(json));
    }else{
        lob_set(tmp,(char *)"end",(char *)"true");
        LOG("pointer delteted=%p",ch);
        delete(ch);
    }
    link_direct(link,tmp,NULL);
    LOG("sent %s",lob_json(tmp));
    lob_free(tmp);
    lob_free(data);
    free(json);
}                                          

void StorjTelehash::serviceOnOpenHandler(link_t link, e3x_channel_t chan,
     void *arg){
    ChannelHandler* ch=(ChannelHandler *)arg;
    lob_t packet = e3x_channel_receiving(chan);
    sendOnChannel(link,chan,packet,ch);
    lob_free(packet);
}

lob_t StorjTelehash::serviceOnOpen(link_t link,lob_t open){
    if(!link || !open) return open;
    char *type=(char *)lob_get(open,(char *)"type");
    ChannelHandlerFactory *factory=factories()[link->mesh->id->hashname];
    ChannelHandler *c=factory->createInstance(type);
    if(!c) return open;
    LOG("pointer=%p",c);

    LOG("openning channel in link_handler() with %s",lob_json(open));
    e3x_channel_t chan=link_channel(link,open);
    link_handle(link, chan, serviceOnOpenHandler,c);
    sendOnChannel(link,chan,open,c);
    return NULL;
}

//ugly. using inner struct.
typedef struct pipe_udp4_struct
{
  struct sockaddr_in sa;
  net_udp4_t net;
} *pipe_udp4_t;

void StorjTelehash::addBroadcasterHandler(link_t link, e3x_channel_t chan,
     void *arg){
    lob_t packet = e3x_channel_receiving(chan);
    strcpy(globalIP,lob_get(packet,(char *)"IP"));
    lob_free(packet);
}

lob_t StorjTelehash::signupOnOpen(link_t link,lob_t open){
    if(!link || !open || lob_get_cmp(open,(char *)"type",
        (char *)"signup"))
       return open;
    lob_t json = lob_new();
    lob_set(json,(char *)"end",(char *)"true");
    lob_set_uint(json,(char *)"c",lob_get_int(open,(char *)"c"));

    if(!lob_get_cmp(open,(char *)"action",(char *)"add")){
        broadcastee.push_back(link);
        tgc_addRoot(link);
        lob_set(json,(char *)"action",(char *)"add");
    }
    if(!lob_get_cmp(open,(char *)"action",(char *)"del")){
        broadcastee.remove(link);
        tgc_rmRoot(link);
        lob_set(json,(char *)"action",(char *)"del");
    }
    
    //ugly. using inner struct.
    pipe_udp4_t pu=(pipe_udp4_t)(link_pipes(link,NULL)->arg);
    char *ip=inet_ntoa(pu->sa.sin_addr);
    LOG("ip %s",ip);
    lob_set(json,(char *)"IP",ip);

    LOG("sent %s",lob_json(json));
    link_direct(link,json,NULL);
    lob_free(json);
    return NULL;
}

lob_t StorjTelehash::broadcastOnOpen(link_t link,lob_t open){
    if(!link || !open || lob_get_cmp(open,(char *)"type",
        (char *)"broadcast"))
       return open;
    lob_t data=lob_get_json(open,(char *)"data");
    char *j=lob_json(data);
    LOG("%s",link->mesh->id->hashname);
    ChannelHandler* broadcastHandler=
        broadcastHandlers()[link->mesh->id->hashname];
    char *json_=broadcastHandler->handle(j);
    if(json_){
        list<link_t>::iterator it = broadcastee.begin();
        while( it != broadcastee.end() ){
            if(link!=*it){
                lob_t json = lob_new();
                lob_set_raw(json,(char *)"data",4,
                            json_,strlen(json_));
                lob_set(json,(char *)"end",(char *)"true");
                lob_set(json,(char *)"type",(char *)"broadcast");
                LOG("sent %s",lob_json(json));
                e3x_channel_t chan=link_channel(*it,json);
                link_flush(*it,chan, json); 
                //json was freed in link_flush()
            }
            it++;
        }
        free(json_);
    }
    lob_free(data);
    return NULL;
}

StorjTelehash::StorjTelehash(int port ,
    ChannelHandlerFactory &factory, ChannelHandler &broadcastHandler){
/*
 * must not use static variables! if not, it crashes 
 * if object would be created globaly.
 */
    lob_t id = util_fjson((char *)"id.json");
    if(!id){
        writeID();
        id = util_fjson((char *)"id.json");
    }

    mesh = mesh_new(0);
    //test use only
    if(port==-9999){
        lob_t secret=mesh_generate(mesh);
        lob_free(secret);
    }else{
        lob_t k=lob_get_json(id,(char *)"keys");
        lob_t s=lob_get_json(id,(char *)"secrets");
        mesh_load(mesh,s,k);
        lob_free(k);
        lob_free(s);
    }
    factories()[mesh->id->hashname] = &factory;
    broadcastHandlers()[mesh->id->hashname]=&broadcastHandler;
    mesh_on_discover(mesh,(char *)"auto",mesh_add); 
    mesh_on_link(mesh, (char *)"onLink", onLink);
    mesh_on_open(mesh,(char *)"service",serviceOnOpen);
    mesh_on_open(mesh,(char *)"signup",signupOnOpen);
    mesh_on_open(mesh,(char *)"broadcast",broadcastOnOpen);

    lob_t p = lob_new();
    if(port>0){
        lob_set_int(p,(char *)"port",port);
    }
    udp4 = net_udp4_new(mesh, p);
    net_udp4_timeout(udp4,100);
//    util_sock_timeout(udp4->server,100);
// cygwin cannot use SO_RCVTIMEO.
    lob_free(p);
    lob_free(id);
    
    count++;
}

ChannelHandlerFactory* StorjTelehash::getChannelHandlerFactory(){
    return factories()[mesh->id->hashname];
}

ChannelHandler* StorjTelehash::getBroadcastHandler(){
    return broadcastHandlers()[mesh->id->hashname];
}

/*
 * location={"keys":{"1a":"aif6foqaligryvtbh4xjomjdcewgt53r3m"},
 *           "paths":[{"type":"udp4","ip":"127.0.0.1","port":45449}]}
*/
link_t StorjTelehash::_link(char *location){
    LOG("%s",location);
    lob_t loc=lob_new();
    lob_head(loc,(uint8_t *)location,strlen(location));

    //key='{"1a":"aipysqnms5nl7vl7vseokubjmj35}'
    lob_t _1a=lob_get_json(loc,(char *)"keys");
    LOG("keys:%s",lob_json(_1a));
    link_t link=link_keys(mesh,_1a);

    //path='{"type":"udp4","ip":"127.0.0.1","port":33592}'
    lob_t path=lob_get_array(loc,(char *)"paths");
    LOG("linking to %s\n",lob_json(path));
    link_path(link,path);
    //critical section. assumes two parties would not call _link()
    //(i.e. call method in this class) at the same time in a test.
    if(!link_up(link)){
        link_sync(link);
        targetLink=link;
        for(status=0;!status;) {net_udp4_receive(udp4);}
        targetLink=NULL;
    }
    lob_free(_1a);
    lob_free(path);
    lob_free(loc);
    return link;
}

void StorjTelehash::openChannel(char *location, char *name,
                                     ChannelHandler &h){
    link_t link=_link(location);
    lob_t options = lob_new();
    lob_set(options,(char *)"type",name);
    char *json=h.handle(NULL);
    lob_set_raw(options,(char *)"data",4,json,strlen(json));
    LOG("openChannel %s",lob_json(options));
    e3x_channel_t chan=link_channel(link,options);
    link_handle(link, chan, serviceOnOpenHandler, &h);
    link_flush(link,chan, options); 
    free(json);
    //options is freed in link_flush
}

void StorjTelehash::addBroadcaster(char *location,int add){
    link_t link=_link(location);
    lob_t options = lob_new();
    lob_set(options,(char *)"type",(char *)"signup");
    if(add){
        lob_set(options,(char *)"action",(char *)"add");
    }else{
        lob_set(options,(char *)"action",(char *)"del");
    }
    lob_set(options,(char *)"end",(char *)"true");
    e3x_channel_t chan=link_channel(link,options);
    link_handle(link, chan, addBroadcasterHandler,NULL);
    link_flush(link,chan, options); 
    //options is freed in link_flush
}


void StorjTelehash::broadcast(char *location, char *json){
    link_t link=_link(location);
    lob_t options = lob_new();
    lob_set(options,(char *)"type",(char *)"broadcast");
    lob_set_raw(options,(char *)"data",4,json,strlen(json));
    lob_set(options,(char *)"end",(char *)"true");
    LOG("sending %s",lob_json(options));
    e3x_channel_t chan=link_channel(link,options);
    link_flush(link,chan, options); 
    //options is freed in link_flush
}

void StorjTelehash::setStopFlag(int flag){
    stopFlag=flag;
}
void StorjTelehash::start(){
    while(!stopFlag && net_udp4_receive(udp4));
}	

int StorjTelehash::_isLocalTest(char *adr){
    return isLocal(adr);
}	


StorjTelehash::~StorjTelehash(){
    net_udp4_free(udp4);
    mesh_free(mesh);
#ifndef __NO_TGC__
    if(--count==0){
        e3x_cipher_free();
        list<link_t>::iterator it = broadcastee.begin();
        while( it != broadcastee.end() ){
            tgc_rmRoot(*it);
            it++;
        }
        gcollect();
    }
#endif
}	

void StorjTelehash::gcollect(){
#ifndef __NO_TGC__
    tgc_gcollect();
#endif
}
