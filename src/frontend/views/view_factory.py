from enum import Enum
from views.gui_view import GuiView
from views.no_gui_console_view import NoGUIConsoleView
from views.view_interface import IView


class ViewMode(Enum):
    """View mode selection"""
    CONSOLE = "console"
    GUI = "gui"


class ViewFactory:
    """Factory for creating view instances"""

    @staticmethod
    def create(mode: ViewMode) -> IView:
        """
        Create a view instance based on the view mode.

        @param mode View mode (CONSOLE or GUI)
        @return View instance
        """
        if mode == ViewMode.GUI:
            return GuiView()
        elif mode == ViewMode.CONSOLE:
            return NoGUIConsoleView()
        else:
            raise ValueError(f"Unsupported view mode: {mode}")