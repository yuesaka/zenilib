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

#include <Zeni/Console_State.h>

#include <Zeni/Colors.h>
#include <Zeni/Fonts.h>
#include <Zeni/Game.h>
#include <Zeni/Video.hxx>
#include <Zeni/Widget.hxx>

using namespace std;

namespace Zeni {

  Console_State::Console_State()
    : m_virtual_screen(Point2f(0.0f, 0.0f), Point2f(float(get_Video().get_screen_width()), 600.0f)),
    m_projector(m_virtual_screen),
    m_bg(Vertex2f_Color(Point2f(0.0f, 0.0f), get_Colors()["console_border"]),
         Vertex2f_Color(Point2f(0.0f, 54.0f + 7.0f * get_Fonts()["system_36_600"].get_text_height()), get_Colors()["console_border"]),
         Vertex2f_Color(Point2f(float(get_Video().get_screen_width()), 54.0f + 7.0f * get_Fonts()["system_36_600"].get_text_height()), get_Colors()["console_border"]),
         Vertex2f_Color(Point2f(float(get_Video().get_screen_width()), 0.0f), get_Colors()["console_border"])),
    m_log(Point2f(18.0f, 18.0f),
          Point2f(get_Video().get_screen_width() - 18.0f,
                  18.0f + 6.0f * get_Fonts()["system_36_600"].get_text_height()),
          get_Colors()["console_background"],
          "system_36_600",
          "",
          get_Colors()["console_foreground"],
          false),
    m_prompt(Point2f(18.0f, 36.0f + 6.0f * get_Fonts()["system_36_600"].get_text_height()),
             Point2f(get_Video().get_screen_width() - 18.0f,
                     36.0f + 7.0f * get_Fonts()["system_36_600"].get_text_height()),
             get_Colors()["console_background"],
             "system_36_600",
             "",
             get_Colors()["console_foreground"],
             true),
    m_child(0)
  {
    m_functions["args"] = new Console_Function;
  }

  Console_State::~Console_State() {
    for(std::map<std::string, Console_Function *>::iterator it = m_functions.begin(); it != m_functions.end(); ++it)
      delete it->second;
  }

  Gamestate & Console_State::get_child() {
    assert(m_child);
    return *m_child;
  }

  void Console_State::set_child(Gamestate child) {
    delete m_child;
    m_child = 0;
    m_child = new Gamestate(child);
  }

  void Console_State::clear_child() {
    delete m_child;
    m_child = 0;
  }

  void Console_State::write_to_log(const std::string &text) {
    if(m_log.get_text().empty())
      m_log.set_text(text);
    else
      m_log.set_text(m_log.get_text() + "\n" + text);

    const int max_lines = m_log.get_max_lines();
    if(m_log.get_num_lines() > max_lines)
      m_log.erase_lines(0, m_log.get_num_lines() - max_lines);
  }

  void Console_State::clear_log() {
    m_log.set_text("");
  }

  void Console_State::give_function(const std::string &name, Console_Function * const &function) {
    assert(function);

    if(m_functions[name])
      delete m_functions[name];
    m_functions[name] = function;
  }

  void Console_State::fire_command(const std::string &text) {
    if(text.empty())
      return;

    std::string command;
    std::vector<std::string> args;

    std::string token;
    bool in_quotes = false;
    bool escaped = false;
    for(std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
      if(escaped) {
        token += *it;
        escaped = false;
      }
      else {
        switch(*it) {
          case '"':
            if(in_quotes || !token.empty()) {
              if(command.empty())
                command = token;
              else
                args.push_back(token);
              token.clear();
            }
            in_quotes = !in_quotes;
            break;

          case ' ':
          case '\t':
            if(in_quotes)
              token += *it;
            else if(!token.empty()) {
              if(command.empty())
                command = token;
              else
                args.push_back(token);
              token.clear();
            }
            break;

          case '\\':
            escaped = true;
            break;

          default:
            token += *it;
            break;
        }
      }
    }

    if(!token.empty())
      if(command.empty())
        command = token;
      else
        args.push_back(token);
    else if(command.empty())
      return;

    std::map<std::string, Console_Function *>::iterator func = m_functions.find(command);
    if(func != m_functions.end())
      (*(func->second))(*this, command, args);
    else
      write_to_log("Unrecognized command: " + command);
  }

  void Console_State::on_event(const SDL_Event &event) {
    m_handled_event = false;
    Gamestate_Base::on_event(event);
    if(!m_handled_event && m_child) {
      assert(m_child);
      m_child->on_event(event);
    }
  }

  void Console_State::on_key(const SDL_KeyboardEvent &event) {
    if(event.type == SDL_KEYDOWN &&
       !(event.keysym.mod & (KMOD_ALT | KMOD_CTRL | KMOD_META)))
    {
      switch(event.keysym.sym) {
        case SDLK_RETURN:
          if(m_prompt.get_edit_pos() != -1) {
            fire_command(m_prompt.get_text());
            m_prompt.set_text("");
          }
          break;

        case SDLK_BACKQUOTE:
          if((event.keysym.mod & KMOD_SHIFT) /*&& m_prompt.get_edit_pos() == -1*/) {
            assert(m_child);

            Game &gr = get_Game();
            gr.pop_state();
            gr.push_state(*m_child);
            clear_child();

            m_handled_event = true;
          }

        default:
          if(m_prompt.get_edit_pos() != -1) {
            const std::string backup_text = m_prompt.get_text();
            const int backup_pos = m_prompt.get_edit_pos();
            const bool restore = m_prompt.get_num_lines() == 1;

            m_prompt.on_event(event);

            if(m_prompt.get_num_lines() > 1 && restore) {
              m_prompt.set_text(backup_text);
              m_prompt.seek(backup_pos);
            }
          }
          break;
      }
    }

    m_handled_event = m_prompt.get_edit_pos() != -1;
  }

  void Console_State::on_mouse_button(const SDL_MouseButtonEvent &event) {
    m_prompt.on_event(event, m_virtual_screen);
    m_handled_event = m_prompt.get_edit_pos() != -1;
  }

  void Console_State::on_mouse_motion(const SDL_MouseMotionEvent &event) {
    m_prompt.on_event(event, m_virtual_screen);
    m_handled_event = m_prompt.get_edit_pos() != -1;
  }

  void Console_State::render() {
    assert(m_child);

    m_child->render();

    get_Video().set_2d(m_virtual_screen);

    get_Video().render(m_bg);
    m_log.render();
    m_prompt.render();
  }

  void Console_Function::operator()(Console_State &console,
                                    const std::string &name,
                                    const std::vector<std::string> &args)
  {
    std::string fargs;

    if(!args.empty()) {
      std::vector<std::string>::const_iterator it = args.begin();
      fargs += *it;
      for(++it; it != args.end(); ++it)
        fargs += ", " + *it;
    }

    console.write_to_log(name + '(' + fargs + ')');
  }

}
