/*
 * Copyright 2010-2015 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "NewResearchListState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Mod/RuleResearch.h"
#include "ResearchInfoState.h"

namespace OpenXcom
{
/**
 * Initializes all the elements in the Research list screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
NewResearchListState::NewResearchListState(Base *base) : _base(base)
{
	_screen = false;

	_window = new Window(this, 230, 140, 45, 30, POPUP_BOTH);
	_btnOK = new TextButton(103, 16, 164, 146);
	_btnShowOnlyNew = new ToggleTextButton(103, 16, 53, 146);
	_txtTitle = new Text(214, 16, 53, 38);
	_lstResearch = new TextList(198, 88, 53, 54);

	// Set palette
	setInterface("selectNewResearch");

	add(_window, "window", "selectNewResearch");
	add(_btnOK, "button", "selectNewResearch");
	add(_btnShowOnlyNew, "button", "selectNewResearch");
	add(_txtTitle, "text", "selectNewResearch");
	add(_lstResearch, "list", "selectNewResearch");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK05.SCR"));

	_btnOK->setText(tr("STR_OK"));
	_btnOK->onMouseClick((ActionHandler)&NewResearchListState::btnOKClick);
	_btnOK->onKeyboardPress((ActionHandler)&NewResearchListState::btnOKClick, Options::keyCancel);
	_btnOK->onKeyboardPress((ActionHandler)&NewResearchListState::btnMarkAllAsSeenClick, Options::keyInvClear);

	_btnShowOnlyNew->setText(tr("STR_SHOW_ONLY_NEW"));
	_btnShowOnlyNew->onMouseClick((ActionHandler)&NewResearchListState::btnShowOnlyNewClick);

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_NEW_RESEARCH_PROJECTS"));

	_lstResearch->setColumns(1, 190);
	_lstResearch->setSelectable(true);
	_lstResearch->setBackground(_window);
	_lstResearch->setMargin(8);
	_lstResearch->setAlign(ALIGN_CENTER);
	_lstResearch->onMouseClick((ActionHandler)&NewResearchListState::onSelectProject, SDL_BUTTON_LEFT);
	_lstResearch->onMouseClick((ActionHandler)&NewResearchListState::onToggleProjectStatus, SDL_BUTTON_RIGHT);
}

/**
 * Initializes the screen (fills the list).
 */
void NewResearchListState::init()
{
	State::init();
	fillProjectList(false);
}

/**
 * Selects the RuleResearch to work on.
 * @param action Pointer to an action.
 */
void NewResearchListState::onSelectProject(Action *)
{
	_game->pushState(new ResearchInfoState(_base, _projects[_lstResearch->getSelectedRow()]));
}

/**
* Selects the RuleResearch to work on.
* @param action Pointer to an action.
*/
void NewResearchListState::onToggleProjectStatus(Action *)
{
	// change status
	const std::string rule = _projects[_lstResearch->getSelectedRow()]->getName();
	int oldState = _game->getSavedGame()->getResearchRuleStatus(rule);
	int newState = 1 - oldState;
	_game->getSavedGame()->setResearchRuleStatus(rule, newState);

	if (newState == RuleResearch::RESEARCH_STATUS_NEW)
	{
		_lstResearch->setRowColor(_lstResearch->getSelectedRow(), 241); // pink
	}
	else
	{
		_lstResearch->setRowColor(_lstResearch->getSelectedRow(), 208); // white
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void NewResearchListState::btnOKClick(Action *)
{
	_game->popState();
}

/**
* Filter to display only new items.
* @param action Pointer to an action.
*/
void NewResearchListState::btnShowOnlyNewClick(Action *)
{
	fillProjectList(false);
}

/**
 * Marks all items as seen
 * @param action Pointer to an action.
 */
void NewResearchListState::btnMarkAllAsSeenClick(Action *)
{
	fillProjectList(true);
}

/**
 * Fills the list with possible ResearchProjects.
 */
void NewResearchListState::fillProjectList(bool markAllAsSeen)
{
	_projects.clear();
	_lstResearch->clearList();
	_game->getSavedGame()->getAvailableResearchProjects(_projects, _game->getMod() , _base);
	std::vector<RuleResearch*>::iterator it = _projects.begin();
	int row = 0;
	bool hasUnseen = false;
	while (it != _projects.end())
	{
		if (_btnShowOnlyNew->getPressed())
		{
			if (_game->getSavedGame()->getResearchRuleStatus((*it)->getName()) != RuleResearch::RESEARCH_STATUS_NEW)
			{
				it = _projects.erase(it);
				continue;
			}
		}
		if ((*it)->getRequirements().empty())
		{
			_lstResearch->addRow(1, tr((*it)->getName()).c_str());
			if (markAllAsSeen)
			{
				// mark all (new) research items as normal
				_game->getSavedGame()->setResearchRuleStatus((*it)->getName(), RuleResearch::RESEARCH_STATUS_NORMAL);
			}
			else if (_game->getSavedGame()->getResearchRuleStatus((*it)->getName()) == RuleResearch::RESEARCH_STATUS_NEW)
			{
				_lstResearch->setRowColor(row, 241); // pink
				hasUnseen = true;
			}
			row++;
			++it;
		}
		else
		{
			it = _projects.erase(it);
		}
	}

	std::wstring label = tr("STR_SHOW_ONLY_NEW");
	_btnShowOnlyNew->setText((hasUnseen ? L"* " : L"") + label);
}

}
