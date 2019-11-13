/*
 * Copyright (c) 2012, 2017, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

// mysql-backup.cpp : Defines the entry point for the console application.

//--------------------------------------------------------------------------------------------------

static void setup_python(int argc, char **argv) {
  char *pathlist = NULL;
  TCHAR szPath[MAX_PATH];
  std::string module_path;
  if (GetModuleFileName(NULL, szPath, MAX_PATH)) {
    CHAR char_path[MAX_PATH] = {0};
    WideCharToMultiByte(CP_UTF8, 0, szPath, -1, char_path, MAX_PATH, NULL, NULL);
    std::string full_path(char_path);
    size_t path_end = full_path.find_last_of('\\');
    if (path_end != std::string::npos)
      module_path = full_path.substr(0, path_end + 1);
  }

#if 0
  if (_dupenv_s(&pathlist, &bufsize, "PYTHONPATH") == 0 && pathlist)
  {
    int len = strlen("PYTHONPATH")+1+strlen(pathlist)+strlen(";python;scripts")+1;
    char *tmp = (char*)malloc(len);
    sprintf_s(tmp, len, "PYTHONPATH=%s;python;scripts", pathlist);
    _putenv(tmp);
    free(tmp);
  }
  else
#endif

  SetDllDirectoryA((module_path + "..").c_str());

  // This should initialize PYTHONPATH, but for some reason it doesn't work when built in VS2010.
  std::string pythonpath;
  pythonpath.append(module_path).append("modules;");
  pythonpath.append(module_path).append("python;");
  pythonpath.append(module_path).append("python\\Lib;");
  pythonpath.append(module_path).append("python\\libs;");
  pythonpath.append(module_path).append("python\\DLLs;");
  _putenv_s("PYTHONHOME", "python");
  _putenv_s("PYTHONPATH", pythonpath.c_str());
  if (pathlist)
    free(pathlist);

  Py_SetProgramName(argv[0]);

  // Here comes some ugly hack to fix PYTHONPATH init problem.
  // Create dummy site package to avoid error import site message from Py_Initialize();
  FILE *pFile;
  pFile = fopen("site.py", "w");
  if (pFile != NULL)
    fclose(pFile);

  Py_Initialize();

  // Delete dummy site module.
  remove("site.py");
  remove("site.pyc");

  PySys_SetArgv(argc, argv);

  // Now import sys and modify PYTHONPATH.
  PyRun_SimpleString("import sys");
  PyRun_SimpleString("sys.path.append('modules')");
  PyRun_SimpleString("sys.path.append('python')");
  PyRun_SimpleString("sys.path.append('python\\Lib')");
  PyRun_SimpleString("sys.path.append('python\\libs')");
  PyRun_SimpleString("sys.path.append('python\\DLLs')");

  // Reload site module with real module from updated PYTHONPATH.
  PyRun_SimpleString("import site");
  PyRun_SimpleString("reload(site)");
}

//--------------------------------------------------------------------------------------------------

static void finalize_python() {
  Py_Finalize();
}

//--------------------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
  // Set the python interpreter.
  setup_python(argc, argv);

  std::string filepath(argv[0]);

  // Get the path to the helper script.
  std::string helper_path;
  size_t path_end = filepath.find_last_of('\\');
  if (path_end != std::string::npos)
    helper_path = filepath.substr(0, path_end + 1);

  helper_path += "wbadminhelper.py";

  // Configures the execution of the script to take the same
  // parameters as this helper tool.
  Py_SetProgramName(argv[0]);

  PySys_SetArgv(argc, argv);

  // Executes the helper script.
  FILE* f = base_fopen(helper_path.c_str(), "r");

  PyRun_SimpleFileEx(f, "wbadminhelper.py", 1);

  finalize_python();

  return 0;
}

//--------------------------------------------------------------------------------------------------
