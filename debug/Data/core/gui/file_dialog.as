string currentTable;
string root;
string datasource_name;

class ResourcePreviewFormatter : IDataFormatter
{
	ResourcePreviewFormatter()
	{
		AddDataFormatter(rString("resource_preview"), this);
		console.println("adding preview formatter");
	}
	~ResourcePreviewFormatter()
	{
		console.println("deleting preview formatter");
	}
	
	rString FormatData(const StringList &in raw_data)
	{
		string formattedData = "";
		if (raw_data.size() >= 2)
		{
			string r0 = raw_data[0];
			string r1 = raw_data[1];
			/*formattedData =
				"<span id=\"r_filename\" style=\"icon-decorator: image; icon-image:" + r0 + ";\" "
				"onmouseover=\"%this:GeneratePreviewPopup('" + r0 + "', event);\" "
				"onmousemove=\"%this:MovePreviewPopup(event);\" "
				"onmouseout=\"%this:HidePreviewPopup(event);\">" +
				r0 +
				"</span>";
			*/
			formattedData =
				"<div class=\"resource_row\"><div class=\"r_filename\">" + r1 + "</div>" +
				"<fileinfo class=\"r_path\" style=\"display: none;\" path=\"" + r0 + "\"/></div>";
		}
		return formattedData;
	}
};

ResourcePreviewFormatter@ previewFormatter = @ResourcePreviewFormatter();

void UpdateFormField(ElementDocument@ doc, const string &in id, const string &in value)
{
	ElementFormControlInput@ element = cast<ElementFormControlInput>( doc.GetElementById(id) );
	if (element !is null)
	{
		element.SetValue(value);
	}
}

void UpdateFilenameFormField(ElementDocument@ doc, const string &in value)
{
	UpdateFormField(doc, "text_filename", value);
}

void UpdatePathFormField(ElementDocument@ doc)
{
	UpdateFormField(doc, "text_path", currentTable);
	UpdateFormField(doc, "path_display", currentTable);
	//~ ElementFormControlInput@ element = cast<ElementFormControlInput>( doc.GetElementById("path_display") );
	//~ if (element !is null)
	//~ {
		//~ element.SetValue(value);
	//~ }
}

void ChangePath(ElementDocument@ doc, const string &in path)
{
	ElementDataGrid@ dataGrid = cast<ElementDataGrid>( doc.GetElementById(rString("file_list")) );
	if (dataGrid !is null)
	{
		dataGrid.SetDataSource(rString(datasource_name + ".'" + path + "'"));

		currentTable = path;
	}
}

string ExtractTableFromDataSource(string filesystem_datasource_string)
{
	int p = filesystem_datasource_string.findFirst(".");
	if (p != -1)
	{
		// Get this for later use
		if (datasource_name == "")
			datasource_name = filesystem_datasource_string.substr(0, p);
		
		return filesystem_datasource.preprocessPath(filesystem_datasource_string.substr(p+1));
	}
	else
		return "";
}

string UpdateCurrentTable(Event@ event)
{
	string dataSource = event.GetParameter(rString("source"), rString(""));
	string result = ExtractTableFromDataSource(dataSource);
	if (currentTable == "")
	{
		root = currentTable;
	}
	currentTable = result;
	return result;
}

void OnWindowLoad(Event@ event)
{
	//@previewFormatter = ResourcePreviewFormatter();
	//AddDataFormatter(rString("resource_preview"), @previewFormatter);
}

void OnWindowUnload(Event@ event)
{
	RemoveDataFormatter(rString("resource_preview"));
}

void OnWindowShow(Event@ event)
{
	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	ElementDataGrid@ dataGrid = cast<ElementDataGrid>( doc.GetElementById(rString("file_list")) );
	
	if (dataGrid !is null)
	{
		string dataSource = dataGrid.GetAttribute(rString("source"), rString(""));
		
		currentTable = ExtractTableFromDataSource(dataSource);
	
		root = filesystem_datasource.preprocessPath(currentTable);
		
		UpdatePathFormField(doc);
	}
	
	//@previewFormatter = ResourcePreviewFormatter();
	//AddDataFormatter(rString("resource_preview"), @previewFormatter);
}

void OnFileSelected(Event@ event)
{
	int index = event.GetParameter(rString("row_index"), -1);
	if (index == -1)
		return;
	
	string table = currentTable;//UpdateCurrentTable(@event);
	if (table == "")
		return;
	
	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	
	if (!filesystem_datasource.isDirectory(table, index))
	{
		// File selected - set the filename textbox
		string name = filesystem_datasource.filename(table, index);
		UpdateFilenameFormField(doc, name);
	}
	else
	{
		// Folder selected
		string path = filesystem_datasource.path(table, index, true);
		ChangePath(doc, path);
	}
	
	// Set the (hidden) path textbox
	UpdatePathFormField(doc);
}

void OnRowDblClick(Event@ event)
{
	int index = event.GetParameter(rString("row_index"), -1);
	if (index == -1)
		return;
	
	string table = currentTable;//UpdateCurrentTable(@event);
	if (table == "")
		return;
	
	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	
	if (filesystem_datasource.isDirectory(table, index))
	{
		string path = filesystem_datasource.path(table, index, true);
		ChangePath(doc, path);
	}
	
	// Set the (hidden) path textbox
	UpdatePathFormField(doc);
}

void OnUpClicked(Event@ event)
{
	if (root != "" && currentTable != root)
	{
		int p = currentTable.findLast('/');
		if (p != -1)
			currentTable = currentTable.substr(0, p);
		
		ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
		ChangePath(doc, currentTable);
		
		UpdatePathFormField(doc);
	}
}

void OnRefreshClicked(Event@ event)
{
	filesystem_datasource.refresh();
}

ElementDocument@ tooltip = null;
Element@ content = null;
void GeneratePreviewPopup(string filename, Event@ event)
{
	ElementDocument@ document = event.GetTargetElement().GetOwnerDocument();
	if (document !is null)
	{
		if (tooltip is null)
		{
			//@tooltip = document.GetContext().LoadDocument(rString("/core/gui/popup.rml"));
			@tooltip = gui.getContext().LoadDocument(rString("/core/gui/popup.rml"));
			//tooltip.SetAttribute(rString('id'), rString("tooltip"));
			@content = document.CreateElement(rString("div"));
			content.SetAttribute(rString('id'), rString('tooltip_content'));
			
			tooltip.GetFirstChild().AppendChild(content);
			//document.GetFirstChild().AppendChild(tooltip);
		}
		tooltip.Show(DocumentFocusFlags::NONE);
		
		string mouse_x = event.GetParameter(rString("mouse_x"), rString());
		string mouse_y = event.GetParameter(rString("mouse_y"), rString());
		
		content.SetAttribute(rString("style"),
			rString("display: block; width: 60px; position: absolute; left: " + mouse_x + "; top: " + mouse_y + ";"));
		
		content.SetInnerRML(rString('<img src="' + currentTable + '/' + filename + '" width="60px" height="60px"/>'));
	}
}

void MovePreviewPopup(Event@ event)
{
	string mouse_x = event.GetParameter(rString("mouse_x"), rString());
	string mouse_y = event.GetParameter(rString("mouse_y"), rString());
	content.SetAttribute(rString("style"),
		rString("display: block; width: 60px; position: absolute; left: " + mouse_x + "; top: " + mouse_y + ";"));
}

void HidePreviewPopup(Event@ event)
{
	ElementDocument@ document = event.GetTargetElement().GetOwnerDocument();
	if (document !is null)
	{
		if (tooltip !is null)
		{
			content.SetAttribute(rString("style"), rString("display: none;"));
			tooltip.Hide();
		}
	}
}
