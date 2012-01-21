string currentTable;
string root;

void OnWindowLoad(Event@ event)
{
}

void OnFileSelected(Event@ event)
{
	int index = event.GetParameter(rString("row_index"), -1);
	if (index == -1)
		return;
	
	string dataSource = event.GetParameter(rString("source"), rString(""));
	string table;
	if (currentTable == "")
	{
		int p = dataSource.findFirst(".");
		if (p != -1)
			table = dataSource.substr(p+1);
		root = filesystem_datasource.preprocessPath(table);
	}
	else
		table = currentTable;
	
	if (table == "")
		return;
	
	if (!filesystem_datasource.isDirectory(table, index))
	{
		string name = filesystem_datasource.filename(table, index);
		
		ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
		ElementFormControlInput@ element = cast<ElementFormControlInput>( doc.GetElementById(rString("text_filename")) );
		if (element !is null)
		{
			element.SetValue(name);
		}
	}
	else
	{
		string path = filesystem_datasource.path(table, index);
		
		ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
		ElementDataGrid@ dataGrid = cast<ElementDataGrid>( doc.GetElementById(rString("file_list")) );
		if (dataGrid !is null)
		{
			dataGrid.SetDataSource(rString("filesystem.'" + path + "'"));

			currentTable = path;
		}
		
		ElementFormControlInput@ element = cast<ElementFormControlInput>( doc.GetElementById(rString("text_path")) );
		if (element !is null)
		{
			element.SetValue(currentTable);
		}
	}
}

void OnRowDblClick(Event@ event)
{
	int index = event.GetParameter(rString("row_index"), -1);
	if (index == -1)
		return;
	
	string dataSource = event.GetParameter(rString("source"), rString(""));
	string table;
	if (currentTable == "")
	{
		int p = dataSource.findFirst(".");
		if (p != -1)
			table = dataSource.substr(p+1);
		root = filesystem_datasource.preprocessPath(table);
	}
	else
		table = currentTable;
	
	if (table == "")
		return;
	
	if (filesystem_datasource.isDirectory(table, index))
	{
		string path = filesystem_datasource.path(table, index);
		
		ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
		ElementDataGrid@ dataGrid = cast<ElementDataGrid>( doc.GetElementById(rString("file_list")) );
		if (dataGrid !is null)
		{
			dataGrid.SetDataSource(rString("filesystem.'" + path + "'"));

			currentTable = path;
		}
	}
}

void OnUpClicked(Event@ event)
{
	if (root != "" && currentTable != root)
	{
		int p = currentTable.findLast('/');
		if (p != -1)
			currentTable = currentTable.substr(0, p);
		
		ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
		ElementDataGrid@ dataGrid = cast<ElementDataGrid>( doc.GetElementById(rString("file_list")) );
		if (dataGrid !is null)
		{
			dataGrid.SetDataSource(rString("filesystem.'" + currentTable + "'"));
		}
		
		ElementFormControlInput@ element = cast<ElementFormControlInput>( doc.GetElementById(rString("text_path")) );
		if (element !is null)
		{
			element.SetValue(currentTable);
		}
	}
}
