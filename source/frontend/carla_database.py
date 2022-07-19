#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Carla plugin database code
# Copyright (C) 2011-2021 Filipe Coelho <falktx@falktx.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# For a full copy of the GNU General Public License see the doc/GPL.txt file.

# ---------------------------------------------------------------------------------------------------------------------
# Imports (Global)

from copy import deepcopy
from subprocess import Popen, PIPE

from PyQt5.QtCore import pyqtSignal, pyqtSlot, Qt, QByteArray, QEventLoop, QThread
from PyQt5.QtGui import QPixmap
from PyQt5.QtWidgets import QApplication, QDialog, QDialogButtonBox, QHeaderView, QTableWidgetItem

# ---------------------------------------------------------------------------------------------------------------------
# Imports (Custom)

import ui_carla_add_jack
import ui_carla_database
import ui_carla_refresh

from carla_backend import *
from carla_shared import *
from carla_utils import getPluginTypeAsString, getPluginCategoryAsString
