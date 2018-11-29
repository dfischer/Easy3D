/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*
*	This file is part of EasyGUI: software for processing and rendering
*   meshes and point clouds.
*
*	EasyGUI is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	EasyGUI is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "window.h"
#include "plugin.h"

#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include "viewer.h"
#include "imgui_fonts_droid_sans.h"
#include "ImGuiHelpers.h"


ImGuiContext* Window::context_ = nullptr;


Window::Window(Viewer* viewer, const std::string& title) 
	: name_(title)
{
	viewer_ = viewer;
	viewer_->windows_.push_back(this);
}


void Window::init() {
	if (!context_) {
		// Setup ImGui binding
		IMGUI_CHECKVERSION();

		context_ = ImGui::CreateContext();

		const char* glsl_version = "#version 150";
		ImGui_ImplGlfw_InitForOpenGL(viewer_->window_, false);
		ImGui_ImplOpenGL3_Init(glsl_version);
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 5.0f;

		// load font
		reload_font();
	}
}

void Window::reload_font(int font_size)
{
	ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
	//io.Fonts->AddFontFromFileTTF("../3rd_party/imgui/misc/fonts/Roboto-Medium.ttf", 26.0f);
	io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data, droid_sans_compressed_size, font_size * hidpi_scaling());
    io.FontGlobalScale = 1.0f / pixel_ratio();
}


void Window::cleanup()
{
	for (auto p : plugins_)
		p->cleanup();
}


void Window::post_resize(int width, int height)
{
	if (context_)
	{
		ImGui::GetIO().DisplaySize.x = float(width);
		ImGui::GetIO().DisplaySize.y = float(height);
	}
}

// Mouse IO
bool Window::mouse_press(int button, int modifier)
{
	ImGui_ImplGlfw_MouseButtonCallback(viewer_->window_, button, GLFW_PRESS, modifier);
	return ImGui::GetIO().WantCaptureMouse;
}

bool Window::mouse_release(int button, int modifier)
{
	return ImGui::GetIO().WantCaptureMouse;
}

bool Window::mouse_move(int mouse_x, int mouse_y)
{
	return ImGui::GetIO().WantCaptureMouse;
}

bool Window::mouse_scroll(double delta_y)
{
	ImGui_ImplGlfw_ScrollCallback(viewer_->window_, 0.f, delta_y);
	return ImGui::GetIO().WantCaptureMouse;
}

// Keyboard IO
bool Window::char_input(unsigned int key)
{
	ImGui_ImplGlfw_CharCallback(nullptr, key);
	return ImGui::GetIO().WantCaptureKeyboard;
}

bool Window::key_press(int key, int modifiers)
{
	ImGui_ImplGlfw_KeyCallback(viewer_->window_, key, 0, GLFW_PRESS, modifiers);
	return ImGui::GetIO().WantCaptureKeyboard;
}

bool Window::key_release(int key, int modifiers)
{
	ImGui_ImplGlfw_KeyCallback(viewer_->window_, key, 0, GLFW_RELEASE, modifiers);
	return ImGui::GetIO().WantCaptureKeyboard;
}


bool Window::draw()
{
    float offset = 0.0f;
    float menu_width = 180.f * menu_scaling();
    ImGui::SetNextWindowPos(ImVec2(offset, offset), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(0.0f, offset), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(menu_width, -1.0f), ImVec2(menu_width, -1.0f));

	static bool _viewer_menu_visible = true;
	ImGui::Begin(
        name_.c_str(), &_viewer_menu_visible,
        ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_MenuBar
        | ImGuiWindowFlags_NoTitleBar
//        | ImGuiWindowFlags_NoResize
//                | ImGuiWindowFlags_AlwaysAutoResize
//                | ImGuiWindowFlags_NoScrollbar
//                | ImGuiWindowFlags_NoScrollWithMouse
//                | ImGuiWindowFlags_NoCollapse
//                | ImGuiWindowFlags_NoSavedSettings
//                    //| ImGuiWindowFlags_NoInputs
	);
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);

	draw_widgets();
	for (auto p : plugins_)
		p->draw();

    ImGui::PopItemWidth();
	ImGui::End();
	return false;
}


void Window::draw_widgets()
{
    static bool show_about = false;
    if (show_about)
     {
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth(), 10));
         ImGui::Begin("About Easy3D", &show_about, ImGuiWindowFlags_AlwaysAutoResize);
         ImGui::Text(
			 "Easy3D is an easy, lightweight, and flexible framework for developing\n"
			 "cross-platform 3D applications. It requires minimum dependencies, i.e.\n"
			 "\t- GLFW (for cross-platform OpenGL context creation) and\n"
			 "\t- ImGui (for GUI creation and event handling,\n"
			 "\n"
			 "Easy3D works on all major operating systems with a decent C++11 capable\n"
			 "compiler, e.g., MacOS (Clang), Linux (GCC or Clang), and Windows (Visual\n"
			 "Studio >= 2015). All dependencies are included and built using CMake.\n"
			 "\n"
		 );
		 ImGui::Separator();
		 ImGui::Text(
			 "\n"
			 "Liangliang Nan\n"
			 "liangliang.nan@gmail.com\n"
			 "https://3d.bk.tudelft.nl/liangliang/\n"
		 );
         ImGui::End();
     }

    // Menu
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
			if (ImGui::MenuItem("Open", "Ctrl+O")) { std::cout << "open" << std::endl; }
            if (ImGui::MenuItem("Save As...", "Ctrl+S")) { std::cout << "save" << std::endl; }

			ImGui::Separator();
			if (ImGui::BeginMenu("Recent Files...")) {
 				ImGui::MenuItem("bunny.ply");
				ImGui::MenuItem("terain.las");
				ImGui::MenuItem("building.obj");
				ImGui::EndMenu();
			}

            ImGui::Separator();
			if (ImGui::MenuItem("Quit", "Alt+F4"))
				glfwSetWindowShouldClose(viewer_->window_, GLFW_TRUE);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Snapshot", NULL))
                std::cout << "snapshot" << std::endl;

            ImGui::Separator();
            if (ImGui::MenuItem("Save Camera State", NULL))
                std::cout << "save camera state" << std::endl;
            if (ImGui::MenuItem("Load Camera State", NULL))
                std::cout << "load camera state" << std::endl;

			ImGui::Separator();
			if (ImGui::BeginMenu("Options"))
			{
				static bool fixed_position = true;
				ImGui::MenuItem("Window Fixed Position", "", &fixed_position);

				if (ImGui::BeginMenu("Background Color"))
				{
					ImGui::ColorPicker3("clear color", (float*)viewer_->background_color()); // Edit 3 floats representing a color
					ImGui::EndMenu();
				}

				static int style_idx = 1;
				if (ImGui::Combo("Window Style", &style_idx, "Classic\0Dark\0Light\0"))
				{
					switch (style_idx)
					{
					case 0: ImGui::StyleColorsClassic(); break;
					case 1: ImGui::StyleColorsDark(); break;
					case 2: ImGui::StyleColorsLight(); break;
					}
				}

				ImGui::EndMenu();
			}

            ImGui::EndMenu();
        }

		if (ImGui::BeginMenu("Select"))
		{
			static int element_idx = 0;
			if (ImGui::Combo("", &element_idx, "None\0Vertex\0Edge\0Face\0"))
			{
				switch (element_idx)
				{
				case 0: {std::cout << "none" << std::endl; }; break;
				case 1: {std::cout << "select Vertex" << std::endl; }; break;
				case 2: {std::cout << "select Edge" << std::endl; }; break;
				case 3: {std::cout << "select Face" << std::endl; }; break;
				}
			}
			ImGui::Separator();
			ImGui::MenuItem("Invert Selected", NULL, false);
			ImGui::MenuItem("Delete Selected", NULL, false);
			ImGui::EndMenu();
		}

        if (ImGui::BeginMenu("Help"))
        {
			ImGui::MenuItem("Viewer", NULL, false);
			ImGui::MenuItem("Shortcut", NULL, false);
			ImGui::Separator();
			ImGui::MenuItem("About", NULL, &show_about);
			ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

	// Workspace
	if (ImGui::CollapsingHeader("Workspace", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float w = ImGui::GetContentRegionAvailWidth();
		float p = ImGui::GetStyle().FramePadding.x;
		if (ImGui::Button("Load##Workspace", ImVec2((w - p) / 2.f, 0)))
		{
			//viewer_->load_scene();
		}
		ImGui::SameLine(0, p);
		if (ImGui::Button("Save##Workspace", ImVec2((w - p) / 2.f, 0)))
		{
			//viewer_->save_scene();
		}
	}

	// Mesh
	if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float w = ImGui::GetContentRegionAvailWidth();
		float p = ImGui::GetStyle().FramePadding.x;
		if (ImGui::Button("Load##Mesh", ImVec2((w - p) / 2.f, 0)))
		{
			//viewer_->open_dialog_load_mesh();
		}
		ImGui::SameLine(0, p);
		if (ImGui::Button("Save##Mesh", ImVec2((w - p) / 2.f, 0)))
		{
			//viewer_->open_dialog_save_mesh();
		}
	}
}



float Window::pixel_ratio()
{
	// Computes pixel ratio for hidpi devices
	int buf_size[2];
	int win_size[2];
	GLFWwindow* window = glfwGetCurrentContext();
	glfwGetFramebufferSize(window, &buf_size[0], &buf_size[1]);
	glfwGetWindowSize(window, &win_size[0], &win_size[1]);
	return (float)buf_size[0] / (float)win_size[0];
}

float Window::hidpi_scaling()
{
	// Computes scaling factor for hidpi devices
	float xscale, yscale;
	GLFWwindow* window = glfwGetCurrentContext();
	glfwGetWindowContentScale(window, &xscale, &yscale);
	return 0.5f * (xscale + yscale);
}
