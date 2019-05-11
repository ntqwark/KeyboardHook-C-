

/*
	Keyboard Hook by AsyncKeyState(int virtual_key);
	Version 1.0

	Author: nt_qwark / Free License
*/


#pragma once

#ifndef KEYBOARD_HOOK
#define KEYBOARD_HOOK

#include <Windows.h>
#include <algorithm>
#include <thread>
#include <list>
#include <vector>

class KeyboardHook
{
public:
	void initialize()
	{
		std::thread hook_thread(&KeyboardHook::hook_func, this);
		hook_thread.detach();
	}

	void add_handle_key(int key, void (*func_callback)())
	{
		HandledKey hk(key, func_callback);
		handled_keys.push_back(hk);
	}

	void add_handle_combo(std::vector<int> keys_array, void (*func_callback)())
	{
		HandledCombo hc(keys_array, func_callback);
		handled_combos.push_back(hc);
	}
private:
	class HandledKey
	{
	public:
		HandledKey(int vk, void (*func)())
		{
			vKey = vk;
			callback = func;
			invoked = false;
		}

		bool invoked;
		int vKey;
		void (*callback)();
	};
	class HandledCombo
	{
	public:
		HandledCombo(std::vector<int> vkeys, void (*callback)())
		{
			invoked = false;
			this->callback = callback;
			keys = vkeys;
		}

		bool invoked;
		std::vector<int> keys;
		void (*callback)();
	};

	bool contains(std::vector<int>& v, int integer)
	{
		for (auto const& element : v)
		{
			if (element == integer)
				return true;
		}

		return false;
	}

	void hook_func()
	{
		while (TRUE)
		{
			if (handled_keys.size() < 1) { Sleep(20); }

			for (int i = 0; i < 255; i++)
			{
				if (GetAsyncKeyState(i))
				{
					if (!contains(pressed, i))
					{
						pressed.push_back(i);
					}
				}
				else
				{
					if (contains(pressed, i))
					{
						for (int j = 0; j < handled_keys.size(); j++)
						{
							if (contains(handled_combos[j].keys, i))
								if (handled_keys[j].vKey == i)
									handled_keys[j].invoked = false;
						}

						if (handled_combos.size() > 0)
						{
							for (int k = 0; k < handled_combos.size(); k++)
							{
								if (contains(handled_combos[k].keys, i))
								{
									handled_combos[k].invoked = false;
								}
							}
						}

						for (int j = 0; j < pressed.size(); j++)
						{
							if (pressed[j] == i)
							{
								pressed.erase(pressed.begin() + j);
							}
						}
					}
				}
			}

			if (pressed.size() > 0)
			{
				for (auto const& element_pressed : pressed)
				{
					for (auto& element_handled : handled_keys)
					{
						if (element_handled.vKey == element_pressed)
						{
							if (!element_handled.invoked)
							{
								std::thread func_thread(element_handled.callback);
								func_thread.detach();

								element_handled.invoked = true;
							}
						}
					}
				}

				if (handled_combos.size() > 0)
				{
					for (int i = 0; i < handled_combos.size(); i++)
					{	
						bool combo_released = false;

						for (int j = 0; j < handled_combos[i].keys.size(); j++)
						{
							if (!contains(pressed, handled_combos[i].keys[j]))
								break;
							else
								if (j == handled_combos[i].keys.size() - 1)
								{
									combo_released = true;
								}
						}

						if (combo_released)
						{
							if (!handled_combos[i].invoked)
							{
								std::thread callback_thr(handled_combos[i].callback);
								callback_thr.detach();

								handled_combos[i].invoked = true;
							}

							combo_released = false;
						}
					}
				}
			}

			Sleep(20);
		}
	}

	bool isInitialized = false;
	std::vector <HandledKey> handled_keys;
	std::vector <HandledCombo> handled_combos;
	std::vector <int> pressed;
};

#endif
