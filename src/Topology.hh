/*
 * Copyright 2015 Applied Research Center for Computer Networks
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QtCore>
#include <vector>

#include "Application.hh"
#include "Loader.hh"
#include "Common.hh"
#include "ILinkDiscovery.hh"
#include "Rest.hh"
#include "RestListener.hh"
#include "AppObject.hh"
#include "json11.hpp"
#include "Switch.hh"
#include "StaticFlowPusher.hh"
#include "DelayManager.hh"
#include <tins/ethernetII.h>
#include <tins/rawpdu.h>
#include <tins/ip.h>
#include <tins/tcp.h>
#include <tins/udp.h>
#include <tins/packet_sender.h>

typedef std::vector< switch_and_port > data_link_route;

class Link : public AppObject {
    switch_and_port source;
    switch_and_port target;
    int weight;
    uint64_t obj_id;

public:
    Link(switch_and_port _source, switch_and_port _target, int _weight, uint64_t _id):
        source(_source), target(_target), weight(_weight), obj_id(_id) {}

    json11::Json to_json() const;
    json11::Json to_floodlight_json() const;
    
    uint64_t id() const;
    switch_and_port src() const { return source; }
    switch_and_port dst() const { return target; }

    friend class Topology;
};

class Topology : public Application, RestHandler {
    Q_OBJECT
    SIMPLE_APPLICATION(Topology, "topology")
public:
    Topology();
    void init(Loader* provider, const Config& config) override;
    ~Topology();

    std::string restName() override {return "topology";}
    bool eventable() override {return true;}
    std::string displayedName() { return "Topology"; }
    std::string page() { return "topology.html"; }
    AppType type() override { return AppType::Application; }
    json11::Json handleGET(std::vector<std::string> params, std::string body) override;

    data_link_route computeRoute(uint64_t from, uint64_t to);

protected slots:
    void linkDiscovered(switch_and_port from, switch_and_port to);
    void linkBroken(switch_and_port from, switch_and_port to);

private:
    struct TopologyImpl* m;
    
    std::vector<Link*> topo;

    SwitchManager* m_switch_manager;
    StaticFlowPusher *m_flow_pusher;
    DelayManager *m_delay_manager;

    Link* getLink(switch_and_port from, switch_and_port to);

    //void setRules(switch_and_port from, switch_and_port to, uint32_t cycles);
    //static void senderThread(Switch *src, uint32_t src_id, uint32_t dst_id, uint32_t src_port, uint32_t dst_port);
};
