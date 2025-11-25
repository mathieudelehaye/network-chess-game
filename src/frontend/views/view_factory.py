"""View factory for creating view instances.

Factory pattern implementation for view layer creation.
"""

from enum import Enum
from views.gui_view import GuiView
from views.no_gui_console_view import NoGUIConsoleView
from views.view_interface import IView


class ViewMode(Enum):
    """View mode selection for display."""

    NOGUI = "no_gui"
    GUI = "gui"


class ViewFactory:
    """Factory for creating view instances.

    Creates appropriate view based on mode selection.
    """

    @staticmethod
    def create(mode: ViewMode) -> IView:
        """Create view instance based on view mode.

        Args:
            mode: View mode (NOGUI or GUI)

        Returns:
            IView: View instance (GuiView or NoGUIConsoleView)

        Raises:
            ValueError: If unsupported view mode provided
        """
        if mode == ViewMode.GUI:
            return GuiView()
        elif mode == ViewMode.NOGUI:
            return NoGUIConsoleView()
        else:
            raise ValueError(f"Unsupported view mode: {mode}")
