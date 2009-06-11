/* This file is part of the Zenipex Library.
* Copyleft (C) 2008 Mitchell Keith Bloch a.k.a. bazald
*
* The Zenipex Library is free software; you can redistribute it and/or 
* modify it under the terms of the GNU General Public License as 
* published by the Free Software Foundation; either version 2 of the 
* License, or (at your option) any later version.
*
* The Zenipex Library is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License 
* along with the Zenipex Library; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 
* 02110-1301 USA.
*
* As a special exception, you may use this file as part of a free software
* library without restriction.  Specifically, if other files instantiate
* templates or use macros or inline functions from this file, or you compile
* this file and link it with other files to produce an executable, this
* file does not by itself cause the resulting executable to be covered by
* the GNU General Public License.  This exception does not however
* invalidate any other reasons why the executable file might be covered by
* the GNU General Public License.
*/

#include <Zeni/Sound_Source_Pool.h>

#include <Zeni/Sound.hxx>
#include <Zeni/Sound_Source.hxx>
#include <Zeni/Coordinate.hxx>
#include <Zeni/Mutex.hxx>
#include <Zeni/Vector3f.hxx>

#include <algorithm>
#include <iostream>

#include <Zeni/Global.h>

using namespace std;

namespace Zeni {

  Sound_Source_Pool::Sound_Source_Pool()
    : m_replacement_policy(new Replacement_Policy()),
    delete_m_replacement_policy(true)
  {
    // Ensure Sound is initialized
    get_Sound();
  }

  Sound_Source_Pool::~Sound_Source_Pool() {
    purge();

    for(std::vector<Sound_Source *>::iterator it = m_handles.begin();
        it != m_handles.end();
        ++it)
    {
      (*it)->m_remove_from_Pool_on_destruction = false;
    }

    if(delete_m_replacement_policy)
      delete m_replacement_policy;
  }

  Sound_Source_Pool & get_Sound_Source_Pool() {
    static Sound_Source_Pool e_Sound_Source_Pool;
    return e_Sound_Source_Pool;
  }

  bool Sound_Source_Pool::Replacement_Policy::operator()(const Sound_Source &lhs, const Sound_Source &rhs) const {
    if(!rhs.is_playing())
      return false;
    if(!lhs.is_playing())
      return true;

    return lhs.get_priority() < rhs.get_priority();
  }

  bool Sound_Source_Pool::Replacement_Policy::operator()(const Sound_Source * const &lhs, const Sound_Source * const &rhs) const {
    return (*this)(*lhs, *rhs);
  }
  
  Sound_Source_Pool::Positional_Replacement_Policy::Positional_Replacement_Policy(const Point3f &listener_position_)
    : listener_position(listener_position_)
  {
  }

  bool Sound_Source_Pool::Positional_Replacement_Policy::operator()(const Sound_Source &lhs, const Sound_Source &rhs) const {
    if(!rhs.is_playing())
      return false;
    if(!lhs.is_playing())
      return true;

    if(rhs.get_priority() < lhs.get_priority())
      return false;
    if(lhs.get_priority() < rhs.get_priority())
      return true;

    return rhs.calculate_gain(listener_position) > lhs.calculate_gain(listener_position);
  }

  const Sound_Source_Pool::Replacement_Policy & Sound_Source_Pool::get_Replacement_Policy() const {
    return *m_replacement_policy;
  }
  
  void Sound_Source_Pool::give_Replacement_Policy(Sound_Source_Pool::Replacement_Policy * const &replacement_policy) {
    set_Replacement_Policy(replacement_policy);
    delete_m_replacement_policy = true;
  }
  
  void Sound_Source_Pool::lend_Replacement_Policy(Sound_Source_Pool::Replacement_Policy * const &replacement_policy) {
    set_Replacement_Policy(replacement_policy);
    delete_m_replacement_policy = false;
  }

  void Sound_Source_Pool::pause_all() {
    for(std::vector<Sound_Source *>::iterator it = m_handles.begin();
        it != m_handles.end();
        ++it)
    {
      if((*it)->is_playing())
        (*it)->pause();
    }
  }

  void Sound_Source_Pool::unpause_all() {
    for(std::vector<Sound_Source *>::iterator it = m_handles.begin();
        it != m_handles.end();
        ++it)
    {
      if((*it)->is_paused())
        (*it)->play();
    }
  }

  void Sound_Source_Pool::purge() {
    for(vector<Sound_Source *>::iterator it = m_playing_and_destroying.begin();
        it != m_playing_and_destroying.end();
        ++it)
    {
      delete *it;
    }
    m_playing_and_destroying.clear();

    for(vector<Sound_Source *>::iterator it = m_handles.begin();
        it != m_handles.end();
        ++it)
    {
      if((*it)->is_assigned()) {
        Sound_Source_HW * hw = (*it)->unassign();
        hw->stop();
        delete hw;
      }
    }
  }

  void Sound_Source_Pool::update() {
    /*** Handle the playing and destroying ***/

    vector<Sound_Source *> keepers;
    keepers.reserve(m_playing_and_destroying.size());
    for(vector<Sound_Source *>::iterator it = m_playing_and_destroying.begin();
        it != m_playing_and_destroying.end();
        ++it)
    {
      if((*it)->is_playing())
        keepers.push_back(*it);
      else
        delete *it;
    }
    m_playing_and_destroying.swap(keepers);

    /*** Do the regular update ***/

    vector<Sound_Source_HW *> unassigned_hw;
    const size_t needed_hw = m_handles.size();
    size_t given_hw = 0;

    /*** Find out how much Sound_Source_HW is needed ***/

    for(vector<Sound_Source *>::iterator it = m_handles.begin();
        it != m_handles.end();
        ++it)
    {
      if((*it)->is_assigned())
        ++given_hw;
    }

    /*** Acquire more Sound_Source_HW if needed (and if possible) ***/

    try {
      while(needed_hw > given_hw
#ifdef TEST_NASTY_CONDITIONS
        && given_hw < NASTY_SOUND_SOURCE_CAP
#endif
        ) {
        unassigned_hw.push_back(new Sound_Source_HW());
        ++given_hw;
      }
    }
    catch(Sound_Source_HW_Init_Failure &)
    {
    }

    /*** Strip Sound_Source_HW from low priority 'Sound_Source's, as needed ***/

    const size_t cut = needed_hw - given_hw;

    if(cut) {
      std::stable_sort(m_handles.begin(), m_handles.end(), *m_replacement_policy);

      for(size_t i = 0; i != cut; ++i) {
        Sound_Source &source = *m_handles[i];

        if(source.is_assigned())
          unassigned_hw.push_back(source.unassign());
      }
    }

    /*** Assign Sound_Source_HW to high priority 'Sound_Source's ***/

    vector<Sound_Source_HW *>::iterator jt = unassigned_hw.begin();

    for(size_t i = cut; i != needed_hw; ++i) {
      Sound_Source &source = *m_handles[i];

      if(!source.is_assigned()) {
        source.assign(**jt);
        ++jt;
      }
    }
  }

  void Sound_Source_Pool::play_and_destroy(Sound_Source * const &sound_source) {
    m_playing_and_destroying.push_back(sound_source);
    sound_source->play();
  }

  void Sound_Source_Pool::set_Replacement_Policy(Sound_Source_Pool::Replacement_Policy * const &replacement_policy) {
    assert(replacement_policy);

    if(delete_m_replacement_policy)
      delete m_replacement_policy;

    m_replacement_policy = replacement_policy;
  }

  void Sound_Source_Pool::insert_Sound_Source(Sound_Source &sound_source) {
    m_handles.push_back(&sound_source);
  }

  void Sound_Source_Pool::remove_Sound_Source(Sound_Source &sound_source) {
    sound_source.stop();

    if(sound_source.m_hw) {
      delete sound_source.m_hw;
      sound_source.m_hw = 0;
    }

    m_handles.erase(std::find(m_handles.begin(), m_handles.end(), &sound_source));
  }

}

#include <Zeni/Global_Undef.h>