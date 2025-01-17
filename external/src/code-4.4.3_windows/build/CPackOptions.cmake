# processed with    configure_file @ONLY   so may need  rather than ${VAR} in some circumstances
# $Id: CPackOptions.cmake.in 1079 2012-05-05 22:17:36Z Freddie Akeroyd $
#
#====================================================================
#  NeXus - A common data format for neutron, x-ray and muon science.
#  
#  CPackOptions.cmake.in for building the NeXus library and applications.
#
# Copyright (C) 2008-2012 NeXus International Advisory Committee (NIAC)
#  
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
# 
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
# 
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free 
#  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
#  MA  02111-1307  USA
#             
#  For further information, see <http://www.nexusformat.org>
#
#
#====================================================================
#

include(CPackComponent)

#set(CPACK_ALL_INSTALL_TYPES Full)
#set(CPACK_COMPONENTS_ALL manual definitions)   # default is all components mentioned

if (NOT WIN32)
    if (${CMAKE_VERSION} VERSION_LESS 2.8.4)
        set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove this line when CMake >= 2.8.4 is required
    endif()
endif()

set (CPACK_PACKAGE_NAME "NeXus")
set (CPACK_PACKAGE_VENDOR "NeXus International Advisory Committee")
set (CPACK_PACKAGE_VERSION_MAJOR "4")
set (CPACK_PACKAGE_VERSION_MINOR "4")
set (CPACK_PACKAGE_VERSION_PATCH "2")
set (CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set (CPACK_PACKAGE_CONTACT "NeXus Developers <nexus-tech@nexusformat.org>")
set (CPACK_PACKAGE_DESCRIPTION_FILE "/Users/chiaracarminati/Downloads/code-4.4.3/cmake_include/nexus_description.txt")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "NeXus - a common format for neutron and X-ray scattering data http://www.nexusformat.org/")
set (CPACK_PACKAGE_FILE_NAME "nexus-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${CPACK_PACKAGE_VERSION_PATCH}")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "nexus-source-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "NeXus Data Format")

set (CPACK_RESOURCE_FILE_LICENSE "/Users/chiaracarminati/Downloads/code-4.4.3/InstallerBits/Licences/COPYING.rtf")
set (CPACK_RESOURCE_FILE_README "/Users/chiaracarminati/Downloads/code-4.4.3/cmake_include/nexus_description.txt")
set (CPACK_RESOURCE_FILE_WELCOME "/Users/chiaracarminati/Downloads/code-4.4.3/cmake_include/WELCOME.txt")
set (CPACK_PACKAGE_ICON "/Users/chiaracarminati/Downloads/code-4.4.3/InstallerBits/nexus.ico")

# we do not have any absolute paths, so do not need DESTDIR
SET(CPACK_SET_DESTDIR "OFF")
SET(CPACK_PACKAGE_RELOCATABLE "true")

# HKLM\Software\\
#set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "nexus")

set(CPACK_SOURCE_IGNORE_FILES 
	  "nexus_spec.in;~$;/.svn/;/.cvsignore/;/CMakeFiles/;/nbproject/;autogen.sh;cmake_install.cmake;Makefile;${CPACK_SOURCE_IGNORE_FILES}") 

if (${CPACK_GENERATOR} STREQUAL "DEB")
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_INSTALL_PREFIX "/usr")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_PACKAGE_NAME "nexus") # used for components
#    set(CPACK_DEBIAN_PACKAGE_DEBUG TRUE)
    set (CPACK_DEBIAN_PACKAGE_NAME "nexus")
    set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
    set (CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
    set (CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://www.nexusformat.org/")
endif()
if (${CPACK_GENERATOR} STREQUAL "RPM")
    set(CPACK_INSTALL_PREFIX "/usr")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
#    set(CPACK_RPM_PACKAGE_PREFIX "/usr")
#    set(CPACK_RPM_PACKAGE_DEBUG TRUE)
    set (CPACK_RPM_PACKAGE_NAME "nexus")
#    set (CPACK_RPM_PACKAGE_ARCHITECTURE "noarch")
    set (CPACK_RPM_PACKAGE_RELEASE "1")
#    set (CPACK_RPM_PACKAGE_REQUIRES "python >= 2.5.0, cmake >= 2.8.8")
    set (CPACK_RPM_PACKAGE_REQUIRES "python >= 2.5.0")
#    set (CPACK_RPM_PACKAGE_PROVIDES "")
    set(CPACK_RPM_COMPONENT_INSTALL ON)
    set(CPACK_RPM_PACKAGE_LICENSE "/Users/chiaracarminati/Downloads/code-4.4.3/InstallerBits/Licences/COPYING.txt")
    set(CPACK_RPM_PACKAGE_URL "http://www.nexusformat.org/")
#    set(CPACK_RPM_CHANGELOG_FILE "")
endif()
if (${CPACK_GENERATOR} STREQUAL "TGZ")
	set(CPACK_MONOLITHIC_INSTALL ON)
    set(CPACK_TGZ_COMPONENT_INSTALL OFF)
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
    set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY ON)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
endif()
if (${CPACK_GENERATOR} STREQUAL "ZIP")
	set(CPACK_MONOLITHIC_INSTALL ON)
    set(CPACK_ZIP_COMPONENT_INSTALL OFF)
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
    set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY ON)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
endif()
if (${CPACK_GENERATOR} STREQUAL "CygwinBinary")
    set(CPACK_CYGWIN_PATCH_NUMBER 1)
endif()
if (${CPACK_GENERATOR} STREQUAL "PackageMaker")
#    set(CPACK_MONOLITHIC_INSTALL ON)
    # 10.4 is "Tiger", component based install needs 10.4 and above
    set(CPACK_OSX_PACKAGE_VERSION 10.4) 
endif()
if (${CPACK_GENERATOR} STREQUAL "Bundle")
endif()
if (${CPACK_GENERATOR} STREQUAL "NSIS")
    ### NSIS component installs seem to trigger download option at the moment, so set monolithic install and a full package name until we can figure out why
	set(CPACK_MONOLITHIC_INSTALL ON)
    set(CPACK_PACKAGE_FILE_NAME "nexus-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
    set(CPACK_NSIS_PACKAGE_NAME "NeXus ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} NeXus")
    set(CPACK_NSIS_HELP_LINK "http://www.nexusformat.org/")
    set(CPACK_NSIS_URL_INFO_ABOUT "http://www.nexusformat.org/")
    set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
    set(CPACK_NSIS_MODIFY_PATH OFF)
##    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "!include '/Users/chiaracarminati/Downloads/code-4.4.3\\InstallerBits\\nsis_install.nsh'")
##    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "!include '/Users/chiaracarminati/Downloads/code-4.4.3\\InstallerBits\\nsis_uninstall.nsh'")
#  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
    set(CPACK_PACKAGE_ICON "/Users/chiaracarminati/Downloads/code-4.4.3\\InstallerBits\\nexus.ico")
	set(CPACK_NSIS_MUI_ICON "/Users/chiaracarminati/Downloads/code-4.4.3\\InstallerBits\\nexus.ico")
	set(CPACK_NSIS_MUI_UNIICON "/Users/chiaracarminati/Downloads/code-4.4.3\\InstallerBits\\nexus.ico")
#	set(CPACK_NSIS_MENU_LINKS "http://www.nexusformat.org/" "NeXus Web Site" "bin\\nxvalidate.bat" "NXvalidate")	
	set(CPACK_NSIS_MENU_LINKS "http://www.nexusformat.org/" "NeXus Web Site")	
	if (1)
		set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
		set(CPACK_NSIS_DEFINES "!define NEXUSDIRENVSUFFIX 64")	
	else()
		set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES32")
		set(CPACK_NSIS_DEFINES "!define NEXUSDIRENVSUFFIX 32")	
	endif()
endif()
	  
#set (CPACK_OUTPUT_CONFIG_FILE)
#set (CPACK_PACKAGE_EXECUTABLES)
#set (CPACK_STRIP_FILES)
#set (CPACK_SOURCE_STRIP_FILES)
#set (CPACK_SOURCE_GENERATOR)
#set (CPACK_SOURCE_OUTPUT_CONFIG_FILE)
#set (CPACK_SOURCE_IGNORE_FILES)

if(WIN32)
    set(NXVALIDATE nxvalidate.bat)
else()
    set(NXVALIDATE nxvalidate)
endif()
#set(CPACK_CREATE_DESKTOP_LINKS "nxbrowse" "nxbrowse")

set(CPACK_PACKAGE_EXECUTABLES "nxbrowse" "NXbrowse")

#cpack_add_component(Runtime
#                    DISPLAY_NAME "Binary Applications"
#                    DESCRIPTION "Binary applications such as nxconvert, nxbrows etc..."
#                    )

#cpack_add_component(Documentation
#                    DISPLAY_NAME "Documentation"
#                    DESCRIPTION "Application Documentation, API and help files."
#                    )

#cpack_add_component(Development
#                    DISPLAY_NAME "Development"
#                    DESCRIPTION "Development libraies and headers."
#                    )

#cpack_add_component(Examples
#                    DISPLAY_NAME "Examples"
#                    DESCRIPTION "Code example files."
#                    )

#cpack_add_component(definitions
#                    DISPLAY_NAME "NeXus NXDL definitions"
#                    DESCRIPTION "Binary applications such as nxconvert, nxbrowse etc..."
#					INSTALL_TYPES Full
#					GROUP definitions_group
#                    )

#cpack_add_component(manual
#                    DISPLAY_NAME "NeXus Documentation"
#                    DESCRIPTION "NeXus User Guide and Reference Documentation with examples."
#					INSTALL_TYPES Full
#					GROUP manual_group
#                    )

#cpack_add_install_type(Full DISPLAY_NAME "Full")

#cpack_add_component_group(definitions_group
#                              DISPLAY_NAME definitions
#                              DESCRIPTION definitions
#                              EXPANDED
#                              BOLD_TITLE)

#cpack_add_component_group(manual_group
#                              DISPLAY_NAME manual
#                              DESCRIPTION manual
#                              EXPANDED
#                              BOLD_TITLE)

#cpack_add_component(Development
#                    DISPLAY_NAME "Development"
#                    DESCRIPTION "Development libraries and headers."
#                    )

#cpack_add_component(Examples
#                    DISPLAY_NAME "Examples"
#                    DESCRIPTION "Code example files."
#                    )
