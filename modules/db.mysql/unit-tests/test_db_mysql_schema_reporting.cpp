/*
 * Copyright (c) 2011, 2018, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms, as
 * designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA 
 */

/*

#include <fstream>
#include <string>

#include "testgrt.h"
#include "grt_test_utility.h"
#include "grts/structs.db.mysql.h"
#include "grts/structs.workbench.physical.h"
#include "grts/structs.workbench.model.reporting.h"
#include "grti/wb_model_reporting.h"
#include "grt/grt_manager.h"
#include "wb_helpers.h"
#include "wb_model.h"

RegisterTemplateFilename(TEST_01_FN, "data/modules_grt/schema_reporting/schema_reporting_test_1.tpl.html");
#include "../../../testing/tut/data/modules_grt/schema_reporting/schema_reporting_test_1.tpl.html.varnames.h"

#define TEST_01_FO "data/modules_grt/schema_reporting/res_schema_reporting_test_1.html"

#define TEST_02_FI "data/modules_grt/schema_reporting/basic_schema_report_test_model.mwb"
#define TEST_02_FO "data/modules_grt/schema_reporting/res_schema_reporting_test_2.html"

using namespace std;


BEGIN_TEST_DATA_CLASS(module_db_mysql_schema_reporting)
public:
  WbModelImpl* module;
  WBTester *wbt;
  TEST_DATA_CONSTRUCTOR(module_db_mysql_schema_reporting)
  {
    wbt = new WBTester();
  }
END_TEST_DATA_CLASS

TEST_MODULE(module_db_mysql_schema_reporting, "DB MySQL: schema reporting");

TEST_FUNCTION(1)
{
  //GRT *grt= grtm.get_grt();

  Template* tpl= Template::GetTemplate(TEST_01_FN, STRIP_WHITESPACE);
  ensure("template ok", tpl!=0);

  TemplateDictionary* main_dict = new TemplateDictionary("schema listing");

  main_dict->SetValue(ksrt1_TITLE, "MySQL Schema Report");

  for(int i= 0; i < 2; i++)
  {
    TemplateDictionary* schema_dict= main_dict->AddSectionDictionary(ksrt1_SCHEMATA);
    schema_dict->SetFormattedValue(ksrt1_SCHEMA_NAME, "Schema_%d", i + 1);

    for(int j= 0; j < 4; j++)
    {
      TemplateDictionary* table_dict= schema_dict->AddSectionDictionary(ksrt1_TABLES);
      table_dict->SetFormattedValue(ksrt1_TABLE_NAME, "Table_%d", j + 1);
    }
  }

  main_dict->SetValue(ksrt1_FOOTER, "Generated by MySQL Workbench.");

  std::string output;
  tpl->Expand(&output, main_dict);

  std::ofstream ofs(TEST_01_FO);
  ofs << output;
}


TEST_FUNCTION(2)
{
//  WBTester wbt;

  wbt->wb->open_document(TEST_02_FI);

  ensure_equals("loaded phys model count",
                wbt->wb->get_document()->physicalModels().count(), 1U);

  db_mysql_CatalogRef catalog=
db_mysql_CatalogRef::cast_from(wbt->wb->get_document()->physicalModels().get(0)->catalog());


  Template* tpl= Template::GetTemplate(TEST_01_FN, STRIP_WHITESPACE);
  ensure("template ok", tpl!=0);

  TemplateDictionary* main_dict = new TemplateDictionary("schema listing");

  main_dict->SetValue(ksrt1_TITLE, "MySQL Schema Report");



  for(unsigned int i= 0; i < catalog->schemata().count(); i++)
  {
    TemplateDictionary* schema_dict= main_dict->AddSectionDictionary(ksrt1_SCHEMATA);
    schema_dict->SetValue(ksrt1_SCHEMA_NAME, catalog->schemata().get(i)->name().c_str());

    for(unsigned int j= 0; j < catalog->schemata().get(i)->tables().count(); j++)
    {
      TemplateDictionary* table_dict= schema_dict->AddSectionDictionary(ksrt1_TABLES);
      table_dict->SetValue(ksrt1_TABLE_NAME, catalog->schemata().get(i)->tables().get(j)->name().c_str());
    }
  }

  main_dict->SetValue(ksrt1_FOOTER, "Generated by MySQL Workbench.");

  string output;
  tpl->Expand(&output, main_dict);

  std::ofstream ofs(TEST_02_FO);
  ofs << output;
}

TEST_FUNCTION(10)
{
  string test_filename= "./test_output";

//  WBTester wbt;

  wbt->wb->open_document(TEST_02_FI);

  ensure_equals("loaded phys model count",
                wbt->wb->get_document()->physicalModels().count(), 1U);

  workbench_physical_ModelRef physicalModel= wbt->wb->get_document()->physicalModels().get(0);


  module= grt::GRT::get()->get_native_module<WbModelImpl>();
  ensure("WbModel module initialization", NULL != module);

  grt::DictRef options(true);
  options.gset("basedir", wbt->wboptions->basedir);//wbt->wb->get_wb_options().get_string("basedir"));
  options.gset("title", "Test Report");
  options.gset("filename", "TestReport");
  options.gset("fk_show_parent_and_child_table", 1);
  options.gset("output_path", test_filename);

  ssize_t res= module->generateReport(physicalModel, options);
  ensure("generateSchemaReport call failed.", res == 1);


}

TEST_FUNCTION(11)
{
  string test_filename= "./test_output";

//  WBTester wbt;

  wbt->wb->open_document(TEST_02_FI);

  module= grt::GRT::get()->get_native_module<WbModelImpl>();
  ensure("WbModel module initialization", NULL != module);

  grt::StringListRef templates(grt::Initialized);
  ssize_t res= module->getAvailableReportingTemplates(templates);
  ensure("getAvailableSchemaReportTemplates call failed.", res == 1);

  ensure("no templates returned.", templates.count() > 0);
}

// Due to the tut nature, this must be executed as a last test always,
// we can't have this inside of the d-tor.
TEST_FUNCTION(99)
{
  delete wbt;
}

END_TESTS
*/