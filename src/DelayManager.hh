#pragma once

#include <QtCore>
#include <QTimer>
#include <chrono>
#include <tins/pdu_allocator.h>
#include <tins/ethernetII.h>
#include <tins/rawpdu.h>
#include <tins/ip.h>
#include <tins/arp.h>
#include <tins/tcp.h>
#include <tins/udp.h>
#include <tins/icmp.h>
#include <tins/packet_sender.h>

#include "Switch.hh"
#include "StaticFlowPusher.hh"
#include "TimeUtils.hh"
#include "Common.hh"
#include "OFMessageHandler.hh"
#include "ILinkDiscovery.hh"
#include "Application.hh"
#include "Loader.hh"
#include "DMP.hh"
#include "LLDP.hh"
#include <iostream>
#include <fstream>

namespace std 
{
	template <typename T>
	class RingArray {
		std::vector<T> array;
		size_t max_size;
		size_t ptr; 
		bool rewrite;

	public:
		RingArray(size_t max_size, bool rewrite): max_size(max_size), ptr(0), rewrite(rewrite) {}
		size_t add(const T&);
		void remove(const size_t idx);
		T get(const size_t idx) const;
		T operator[](const size_t idx) const;

		template <typename C> size_t add(const C*);
	};
}

using std::mutex;
using std::RingArray;

namespace dm {
	struct Link
	{
		switch_and_port from;
		switch_and_port to;
		char name[128];

		Link(switch_and_port &from, switch_and_port &to) : from(from), to(to) {
			sprintf(name, "s%lup%u-s%lup%u.txt", from.dpid, from.port, to.dpid, to.port);
		}

		void write(char *msg) {
			std::ofstream out(name, std::ofstream::app);
			if (out.is_open()) {
				std::cout << "File opened!\n";
			}
			else {
				std::cout << "Couldn't open file!\n";
			}
			out << msg << std::endl;
			out.close();
		}
	};
}

void getIpForCycle(uint32_t cycle, char *buf);
void getEthForSwitchPair(size_t s1, size_t s2, char *buf);
void getEth(uint16_t link_idx, uint32_t cycle, char *buf);

class DelayManager: public Application
{
	Q_OBJECT
    SIMPLE_APPLICATION(DelayManager, "delay-manager")
public:
	void init(Loader* loader, const Config& config) override;

	long getLinkIdBySwitch(switch_and_port &src);
	dm::Link* getLinkById(size_t id);
	void setLinkBaseRTT(size_t id, double rtt) { baseLinkRTTs[id] = rtt; }

	pair<size_t, hr_clock_t> requestPacketOut(size_t link);
	void observeLink(switch_and_port from, switch_and_port to);

public slots:
    void onDmpReceived(unsigned short link_id, unsigned pkt_id, unsigned long long t0, unsigned long long t1);

private:
	size_t cycles;

	HashMap<size_t, Lock> loopLock;
	mutex addLinkMutex;

	HashMap<size_t, mutex> linkMutex;
	HashMap<size_t, mutex> timeLineMutex;

	HashMap<size_t, double> baseLinkRTTs;
	HashMap<size_t, TimeRequestLoop*> loops;
	HashMap<size_t, TimeLine*>        timelines;
	HashMap<size_t, HashMap<size_t, size_t> > link_ids;
	HashMap<size_t, std::ofstream> delay_logs;

	Lock linksLock;
	RingArray<dm::Link *> links{1000, true};

	SwitchManager    *switchManager;
    StaticFlowPusher *flowPusher;

    void launchObserver(switch_and_port from, switch_and_port to, size_t link);
    size_t addLink(switch_and_port &from, switch_and_port &to);
	void loadEthRules(size_t link_idx);
	void addLoop(size_t link_idx);
	void addTimeLine(size_t link_idx);

	void loadEth(size_t from, size_t to, size_t cycles);
	void getEthExperimental(size_t link_idx, size_t addr, char *dst);
};