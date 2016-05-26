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

/** @file Common.h
  * @brief Common headers and global definitions.
  */
#pragma once

#include <memory>
#include <glog/logging.h>
#include <QtCore>
#include <fluid/OFConnection.hh>
#include <fluid/of13msg.hh>
#include <mutex>
#include <map>
#include <unordered_map>
#include <list>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

using namespace fluid_base;
using namespace fluid_msg;

#ifndef NDEBUG
#define SMART_CAST static_cast
#else
#define SMART_CAST dynamic_cast
#endif

#define FORMAT_DPID std::hex << std::setw(16) << std::setfill('0')

#define EXISTS(a, b) (a.find(b) != a.end())
#define EXIST2(a, b, c) (EXISTS(a, b) && EXISTS(a[b], c))

namespace std 
{
	template<typename K, typename V> using HashMap = unordered_map<K, V>;
	template<typename T> using ArrayList = vector<T>;
}

typedef boost::shared_mutex Lock;
typedef boost::unique_lock< Lock > WriteLock;
typedef boost::shared_lock< Lock > ReadLock;
using std::HashMap;
using std::ArrayList;

Q_DECLARE_METATYPE(uint32_t)
Q_DECLARE_METATYPE(uint64_t)
Q_DECLARE_METATYPE(of13::FeaturesReply)
Q_DECLARE_METATYPE(of13::PortStatus)
Q_DECLARE_METATYPE(of13::Port)
Q_DECLARE_METATYPE(of13::Match)
Q_DECLARE_METATYPE(std::shared_ptr<of13::Error>)
Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
