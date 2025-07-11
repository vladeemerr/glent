#include "input.hpp"

#include <bitset>
#include <unordered_map>

#include <GLFW/glfw3.h>

namespace glint::input {

namespace mouse {

namespace {

std::bitset<static_cast<size_t>(mouse::Button::count)> states, cached_states;
glm::vec2 cursor_position;
glm::vec2 scroll_delta;

} // namespace

bool isButtonDown(Button button) {
	return states[static_cast<size_t>(button)];
}

bool isButtonPressed(Button button) {
	size_t index = static_cast<size_t>(button);
	return states[index] && !cached_states[index];
}

glm::vec2 cursorPosition() {
	return cursor_position;
}

glm::vec2 scrollDelta() {
	return scroll_delta;
}

} // namespace mouse

namespace keyboard {

namespace {

std::bitset<static_cast<size_t>(keyboard::Key::count)> states, cached_states;

} // namespace

bool isKeyDown(Key key) {
	return states[static_cast<size_t>(key)];
}

bool isKeyPressed(Key key) {
	size_t index = static_cast<size_t>(key);
	return states[index] && !cached_states[index];
}

} // namespace keyboard

void setup(void* const window_ptr) {
	GLFWwindow* window = static_cast<GLFWwindow*>(window_ptr);
	
	glfwSetKeyCallback(window, [](GLFWwindow*, int key, int /*scancode*/,
	                              int action, int /*mods*/) {
		static const std::unordered_map<int, keyboard::Key> keyMap{
			{GLFW_KEY_ESCAPE, keyboard::Key::escape},
			{GLFW_KEY_F1, keyboard::Key::f1},
			{GLFW_KEY_F2, keyboard::Key::f2},
			{GLFW_KEY_F3, keyboard::Key::f3},
			{GLFW_KEY_F4, keyboard::Key::f4},
			{GLFW_KEY_F5, keyboard::Key::f5},
			{GLFW_KEY_F6, keyboard::Key::f6},
			{GLFW_KEY_F7, keyboard::Key::f7},
			{GLFW_KEY_F8, keyboard::Key::f8},
			{GLFW_KEY_F9, keyboard::Key::f9},
			{GLFW_KEY_F10, keyboard::Key::f10},
			{GLFW_KEY_F11, keyboard::Key::f11},
			{GLFW_KEY_F12, keyboard::Key::f12},
			{GLFW_KEY_GRAVE_ACCENT, keyboard::Key::grave},
			{GLFW_KEY_1, keyboard::Key::d1},
			{GLFW_KEY_2, keyboard::Key::d2},
			{GLFW_KEY_3, keyboard::Key::d3},
			{GLFW_KEY_4, keyboard::Key::d4},
			{GLFW_KEY_5, keyboard::Key::d5},
			{GLFW_KEY_6, keyboard::Key::d6},
			{GLFW_KEY_7, keyboard::Key::d7},
			{GLFW_KEY_8, keyboard::Key::d8},
			{GLFW_KEY_9, keyboard::Key::d9},
			{GLFW_KEY_0, keyboard::Key::d0},
			{GLFW_KEY_MINUS, keyboard::Key::minus},
			{GLFW_KEY_EQUAL, keyboard::Key::equal},
			{GLFW_KEY_BACKSPACE, keyboard::Key::backspace},
			{GLFW_KEY_Q, keyboard::Key::q},
			{GLFW_KEY_W, keyboard::Key::w},
			{GLFW_KEY_E, keyboard::Key::e},
			{GLFW_KEY_R, keyboard::Key::r},
			{GLFW_KEY_T, keyboard::Key::t},
			{GLFW_KEY_Y, keyboard::Key::y},
			{GLFW_KEY_U, keyboard::Key::u},
			{GLFW_KEY_I, keyboard::Key::i},
			{GLFW_KEY_O, keyboard::Key::o},
			{GLFW_KEY_P, keyboard::Key::p},
			{GLFW_KEY_LEFT_BRACKET, keyboard::Key::left_bracket},
			{GLFW_KEY_RIGHT_BRACKET, keyboard::Key::right_bracket},
			{GLFW_KEY_BACKSPACE, keyboard::Key::backspace},
			{GLFW_KEY_CAPS_LOCK, keyboard::Key::caps_lock},
			{GLFW_KEY_A, keyboard::Key::a},
			{GLFW_KEY_S, keyboard::Key::s},
			{GLFW_KEY_D, keyboard::Key::d},
			{GLFW_KEY_F, keyboard::Key::f},
			{GLFW_KEY_G, keyboard::Key::g},
			{GLFW_KEY_H, keyboard::Key::h},
			{GLFW_KEY_J, keyboard::Key::j},
			{GLFW_KEY_K, keyboard::Key::k},
			{GLFW_KEY_L, keyboard::Key::l},
			{GLFW_KEY_SEMICOLON, keyboard::Key::semicolon},
			{GLFW_KEY_APOSTROPHE, keyboard::Key::apostrophe},
			{GLFW_KEY_ENTER, keyboard::Key::enter},
			{GLFW_KEY_LEFT_SHIFT, keyboard::Key::left_shift},
			{GLFW_KEY_Z, keyboard::Key::z},
			{GLFW_KEY_X, keyboard::Key::x},
			{GLFW_KEY_C, keyboard::Key::c},
			{GLFW_KEY_V, keyboard::Key::v},
			{GLFW_KEY_B, keyboard::Key::b},
			{GLFW_KEY_N, keyboard::Key::n},
			{GLFW_KEY_M, keyboard::Key::m},
			{GLFW_KEY_COMMA, keyboard::Key::comma},
			{GLFW_KEY_PERIOD, keyboard::Key::period},
			{GLFW_KEY_SLASH, keyboard::Key::slash},
			{GLFW_KEY_RIGHT_SHIFT, keyboard::Key::right_shift},
			{GLFW_KEY_LEFT_CONTROL, keyboard::Key::left_control},
			{GLFW_KEY_LEFT_ALT, keyboard::Key::left_alt},
			{GLFW_KEY_SPACE, keyboard::Key::space},
			{GLFW_KEY_RIGHT_ALT, keyboard::Key::right_alt},
			{GLFW_KEY_RIGHT_CONTROL, keyboard::Key::right_control},
			{GLFW_KEY_INSERT, keyboard::Key::insert},
			{GLFW_KEY_HOME, keyboard::Key::home},
			{GLFW_KEY_PAGE_UP, keyboard::Key::page_up},
			{GLFW_KEY_DELETE, keyboard::Key::kdelete},
			{GLFW_KEY_END, keyboard::Key::end},
			{GLFW_KEY_PAGE_DOWN, keyboard::Key::page_down},
			{GLFW_KEY_LEFT, keyboard::Key::left},
			{GLFW_KEY_UP, keyboard::Key::up},
			{GLFW_KEY_DOWN, keyboard::Key::down},
			{GLFW_KEY_RIGHT, keyboard::Key::right},
		};

		const auto search = keyMap.find(key);

		if (search == keyMap.end())
			return;

		size_t index = static_cast<size_t>(search->second);

		switch (action) {
			case GLFW_PRESS:
				keyboard::states[index] = true;
				break;

			case GLFW_RELEASE:
				keyboard::states[index] = false;
				break;
		}
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action,
	                                      int /*mods*/) {
		size_t index;

		switch (button) {
			case GLFW_MOUSE_BUTTON_LEFT:
				index = static_cast<size_t>(mouse::Button::left);
				break;

			case GLFW_MOUSE_BUTTON_MIDDLE:
				index = static_cast<size_t>(mouse::Button::middle);
				break;

			case GLFW_MOUSE_BUTTON_RIGHT:
				index = static_cast<size_t>(mouse::Button::right);
				break;

			default:
				return;
		}

		switch (action) {
			case GLFW_PRESS:
				mouse::states[index] = true;
				break;

			case GLFW_RELEASE:
				mouse::states[index] = false;
				break;
		}
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow*, double x, double y) {
		mouse::cursor_position.x = x;
		mouse::cursor_position.y = y;
	});

	glfwSetScrollCallback(window, [](GLFWwindow*, double x, double y) {
		mouse::scroll_delta.x = x;
		mouse::scroll_delta.y = y;
	});
}

void cache() {
	keyboard::cached_states = keyboard::states;
	mouse::cached_states = mouse::states;

	mouse::scroll_delta.x = 0.0f;
	mouse::scroll_delta.y = 0.0f;
}

} // namespace glint::input
