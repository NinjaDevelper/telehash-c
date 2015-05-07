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

#include "picojson.h"
#include <stdio.h>
#include <string.h>
#include <tap.h>
#include <vector>  
#include <string>
#include <thread>

#include "MessagingTelehash.h"


using namespace std;
using namespace picojson;

int status=0;
int status1=0;
//m1 cannot connect to m2 because they share key
MessagingTelehash m1(0);
MessagingTelehash m2(1234);
MessagingTelehash m3(-9999);
MessagingTelehash m4(-9999);
string location;
thread t_daemon1,t_daemon2,t_daemon3;
    
char *handler1(char *json){
    static int n=0;
    char *message=NULL;
   
    if(n==0){
        message=(char *)malloc(256);
        strcpy(message,"{\"count\":0}");
        status++;
        n++;
    }else{
        if(n==1){
            message=(char *)malloc(256);
            value v1;
            parse(v1, json);
            double count=v1.get<object>()["count"].get<double>();
            ok(count==0,"channel message test 0");
            strcpy(message,"{\"count\":1}");
            status++;
            n++;
        }else{
            if(n>1){
                status=-9999;
            }
        }
    }
    return message;
}


char *handler2(char *json){
    static int n=0;

    if(n==0){
        value v1;
        parse(v1, json);
        double count=v1.get<object>()["count"].get<double>();
        ok(count==1,"channel message test 1");
        status++;
        n++;
    }else{
        status=-9999;
    }    
   
    return NULL;
}



char *broadcastHandler(char *json){
    LOG("json in=%s",json);
    status++;
    value v1;
    parse(v1, json);
    string service=v1.get<object>()["service"].get<string>();
    ok( service=="farming","broadcast data check.");
    if(status>1) return NULL;
    char *r=(char *)malloc(256);
    strcpy(r,json);
    return r;
}

void locationTest(){
    m1.setGC(0);
    m2.setGC(0);
    m3.setGC(0);
    m4.setGC(0);
    char *info1=m1.getMyLocation();
    char *info2=m2.getMyLocation();
    LOG("info %s",info2);
    value v1,v2;
    parse(v1, info1);
    parse(v2, info2);
    
    string h1=v1.get<object>()["hashname"].get<string>();
    string h2=v2.get<object>()["hashname"].get<string>();
    LOG("hashname=%s",h1.c_str());
    ok( h1==h2,"hashname check.");

    string k1=v1.get<object>()["keys"].get<object>()["1a"].get<string>();
    string k2=v2.get<object>()["keys"].get<object>()["1a"].get<string>();
    LOG("keys=%s",k1.c_str());
    ok( k1==k2,"keys check.");

    int p2=v2.get<object>()["paths"].get<picojson::array>()[0].
            get<object>()["port"].get<double>();
    LOG("port=%d",p2);
    ok( p2==1234,"port check.");
    
    ok( m1._isLocalTest((char *)"192.168.1.1"),"isLocal() check 1.");
    ok( m1._isLocalTest((char *)"172.16.22.22"),"isLocal() check 2.");
    ok( m1._isLocalTest((char *)"172.22.22.22"),"isLocal() check 3.");
    ok( !m1._isLocalTest((char *)"172.32.22.22"),"isLocal() check 4.");
    ok( !m1._isLocalTest((char *)"172.15.22.22"),"isLocal() check 5.");

    location=
        "{\"keys\":{\"1a\":\""+k2+"\"},\"paths\":[{\"type\":\"udp4\",\"ip\""
        ":\"127.0.0.1\",\"port\":1234}]}";
    free(info1);
    free(info2);

}

void singleBroadcasteeTest(){
    LOG("location=%s",location.c_str());
    
    t_daemon1=std::thread([&](){
        m2.start();
    });
    m3.addBroadcaster((char *)location.c_str(),1);
    t_daemon2=std::thread([&](){
        m3.start();
    });
    sleep(2);
    m2.stop();
    m3.stop();
    t_daemon1.join();
    t_daemon2.join();
    LOG("global IP=%s",m3.globalIP);
    ok( !strcmp(m3.globalIP,"127.0.0.1"),"addBroadcaster check.");
    MessagingTelehash::gcollect();

    m3.setBroadcastHandler(broadcastHandler);
    
    t_daemon1=std::thread([&](){
        m2.start();
    });
    status=0;
    m3.broadcast((char *)location.c_str(),(char *)"{\"service\":\"farming\"}");
    t_daemon2=std::thread([&](){
        m3.start();
    });
    sleep(2);
    m2.stop();
    m3.stop();
    t_daemon1.join();
    t_daemon2.join();
    ok( status==1,"broadcaster does not receive his json check.");
    MessagingTelehash::gcollect();
}

void multiBroadcasteeTest(){
    m4.setBroadcastHandler(broadcastHandler);

    t_daemon1=std::thread([&](){
            m2.start();
    });
    m4.addBroadcaster((char *)location.c_str(),1);
    t_daemon2=std::thread([&](){
        m4.start();
    });
    sleep(2);
    m2.stop();
    m4.stop();
    t_daemon1.join();
    t_daemon2.join();
    MessagingTelehash::gcollect();

    t_daemon1=std::thread([&](){
        m4.start();
    });
    t_daemon3=std::thread([&](){
        m2.start();
    });
    status=0;
    m3.broadcast((char *)location.c_str(),(char *)"{\"service\":\"farming\"}");
    t_daemon2=std::thread([&](){
        m3.start();
    });
    sleep(2);
    m2.stop();
    m3.stop();
    m4.stop();
    t_daemon1.join();
    t_daemon2.join();
    t_daemon3.join();
    ok( status==2,"broadcastee receives another's json check.");
    MessagingTelehash::gcollect();
}


void channelTest(){
    vector<CHANNEL_HANDLER> h;
    h.push_back(handler1);
    h.push_back(handler2);
    
    m3.setChannelHandlers((char *)"counter_test",h);

    status=0;
    t_daemon1=std::thread([&](){
        m2.start();
    });
    m3.openChannel((char *)location.c_str(),(char *)"counter_test");
    t_daemon2=std::thread([&](){
        m3.start();
    });
    sleep(2);
    m2.stop();
    m3.stop();
    t_daemon1.join();
    t_daemon2.join();
    LOG("status=%d",status);
    ok( status==3,"channel check.");
    tgc_gcollect();
}

int main (int argc, char *argv[]) {

    locationTest();
    singleBroadcasteeTest();
    multiBroadcasteeTest();
    channelTest();

    done_testing();

}
