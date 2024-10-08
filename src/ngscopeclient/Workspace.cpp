/***********************************************************************************************************************
*                                                                                                                      *
* ngscopeclient                                                                                                        *
*                                                                                                                      *
* Copyright (c) 2012-2024 Andrew D. Zonenberg and contributors                                                         *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

/**
	@file
	@author Andrew D. Zonenberg
	@brief Implementation of Workspace
 */
#include "ngscopeclient.h"
#include "Workspace.h"
#include "Session.h"
#include "MainWindow.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

Workspace::Workspace(Session& session, MainWindow* parent)
	: m_session(session)
	, m_parent(parent)
	, m_open(true)
	, m_title("New Workspace")
	, m_defaultSize(800, 600)
{
	//Assign a new stable ID from the session table
	auto id = session.m_idtable.emplace(this);
	m_id = string("Workspace ") + to_string(id);
}

Workspace::Workspace(const YAML::Node& node, Session& session, MainWindow* parent)
	: m_session(session)
	, m_parent(parent)
	, m_open(true)
	, m_defaultSize(800, 600)
{
	//Reuse, and reserve, the stable ID
	auto id = node["id"].as<uintptr_t>();
	m_session.m_idtable.emplace(id, this);
	m_id = string("Workspace ") + to_string(id);

	m_title = node["title"].as<string>();

	string tmp = m_title + "###" + m_id;
	LogTrace("ID is %s\n", tmp.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serialization

YAML::Node Workspace::Serialize()
{
	YAML::Node ret;
	ret["id"] = m_session.m_idtable[this];
	ret["title"] = m_title;
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Rendering

bool Workspace::Render()
{
	//Closed, nothing to do
	if(!m_open)
		return false;

	auto id = ImGui::GetID(m_id.c_str());

	string name = m_title + "###" + m_id;
	ImGui::SetNextWindowSize(m_defaultSize, ImGuiCond_Appearing);
	if(!ImGui::Begin(name.c_str(), &m_open, ImGuiWindowFlags_NoCollapse))
	{
		TitleHoverHelp();

		//If we get here, the window is tabbed out or the content area is otherwise not visible.
		//Need to keep the dockspace node alive still, though!
		ImGui::DockSpace(id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_KeepAliveOnly, nullptr);

		ImGui::End();
		return true;
	}

	if(ImGui::BeginPopupContextItem())
	{
		ImGui::InputText("Name", &m_title);
		ImGui::EndPopup();
	}

	TitleHoverHelp();

	DoRender(id);

	ImGui::DockSpace(id, ImVec2(0.0f, 0.0f), 0, nullptr);

	ImGui::End();
	return true;
}

void Workspace::TitleHoverHelp()
{
	if(ImGui::IsItemHovered())
	{
		m_parent->AddStatusHelp("mouse_lmb_drag", "Move workspace");
		m_parent->AddStatusHelp("mouse_rmb", "Rename workspace");
	}
}
