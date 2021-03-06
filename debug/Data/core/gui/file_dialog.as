string currentTable;
string root;
string datasource_name;

class ResourcePreviewFormatter : IDataFormatter
{
	ResourcePreviewFormatter()
	{
		AddDataFormatter(Rocket::String("resource_preview"), this);
		console.println("adding preview formatter");
	}
	~ResourcePreviewFormatter()
	{
		console.println("deleting preview formatter");
	}
	
	Rocket::String FormatData(const StringList &in raw_data)
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

void UpdateFormField(Rocket::ElementDocument@ doc, const string &in id, const string &in value)
{
	Rocket::ElementFormControlInput@ element = cast<Rocket::ElementFormControlInput>( doc.GetElementById(id) );
	if (element !is null)
	{
		element.SetValue(value);
	}
}

void UpdateFilenameFormField(Rocket::ElementDocument@ doc, const string &in value)
{
	UpdateFormField(doc, "text_filename", value);
}

void UpdatePathFormField(Rocket::ElementDocument@ doc)
{
	UpdateFormField(doc, "text_path", currentTable);
	UpdateFormField(doc, "path_display", currentTable);
	//~ Rocket::ElementFormControlInput@ element = cast<Rocket::ElementFormControlInput>( doc.GetElementById("path_display") );
	//~ if (element !is null)
	//~ {
		//~ element.SetValue(value);
	//~ }
}

void ChangePath(Rocket::ElementDocument@ doc, const string &in path)
{
	Rocket::ElementDataGrid@ dataGrid = cast<Rocket::ElementDataGrid>( doc.GetElementById(Rocket::String("file_list")) );
	if (dataGrid !is null)
	{
		dataGrid.SetDataSource(Rocket::String(datasource_name + ".'" + path + "'"));

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

string UpdateCurrentTable(Rocket::Event@ event)
{
	string dataSource = event.GetParameter(Rocket::String("source"), Rocket::String(""));
	string result = ExtractTableFromDataSource(dataSource);
	if (currentTable == "")
	{
		root = currentTable;
	}
	currentTable = result;
	return result;
}

void OnWindowLoad(Rocket::Event@ event)
{
	//@previewFormatter = ResourcePreviewFormatter();
	//AddDataFormatter(Rocket::String("resource_preview"), @previewFormatter);
}

void OnWindowUnload(Rocket::Event@ event)
{
	RemoveDataFormatter(Rocket::String("resource_preview"));
}

void OnWindowShow(Rocket::Event@ event)
{
	Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	Rocket::ElementDataGrid@ dataGrid = cast<Rocket::ElementDataGrid>( doc.GetElementById(Rocket::String("file_list")) );
	
	if (dataGrid !is null)
	{
		string dataSource = dataGrid.GetAttribute(Rocket::String("source"), Rocket::String(""));
		
		currentTable = ExtractTableFromDataSource(dataSource);
	
		root = filesystem_datasource.preprocessPath(currentTable);
		
		UpdatePathFormField(doc);
	}
	
	//@previewFormatter = ResourcePreviewFormatter();
	//AddDataFormatter(Rocket::String("resource_preview"), @previewFormatter);
}

void OnFileSelected(Rocket::Event@ event)
{
	int index = event.GetParameter(Rocket::String("row_index"), -1);
	if (index == -1)
		return;
	
	string table = currentTable;//UpdateCurrentTable(@event);
	if (table == "")
		return;
	
	Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	
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

void OnRowDblClick(Rocket::Event@ event)
{
	int index = event.GetParameter(Rocket::String("row_index"), -1);
	if (index == -1)
		return;
	
	string table = currentTable;//UpdateCurrentTable(@event);
	if (table == "")
		return;
	
	Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	
	if (filesystem_datasource.isDirectory(table, index))
	{
		string path = filesystem_datasource.path(table, index, true);
		ChangePath(doc, path);
	}
	
	// Set the (hidden) path textbox
	UpdatePathFormField(doc);
}

void OnUpClicked(Rocket::Event@ event)
{
	if (root != "" && currentTable != root)
	{
		int p = currentTable.findLast('/');
		if (p != -1)
			currentTable = currentTable.substr(0, p);
		
		Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
		ChangePath(doc, currentTable);
		
		UpdatePathFormField(doc);
	}
}

void OnRefreshClicked(Rocket::Event@ event)
{
	filesystem_datasource.refresh();
}

Rocket::ElementDocument@ tooltip = null;
Rocket::Element@ content = null;
void GeneratePreviewPopup(string filename, Rocket::Event@ event)
{
	Rocket::ElementDocument@ document = event.GetTargetElement().GetOwnerDocument();
	if (document !is null)
	{
		if (tooltip is null)
		{
			//@tooltip = document.GetContext().LoadDocument(Rocket::String("/Data/core/gui/popup.rml"));
			@tooltip = gui.getContext().LoadDocument(Rocket::String("/Data/core/gui/popup.rml"));
			//tooltip.SetAttribute(Rocket::String('id'), Rocket::String("tooltip"));
			@content = document.CreateElement(Rocket::String("div"));
			content.SetAttribute(Rocket::String('id'), Rocket::String('tooltip_content'));
			
			tooltip.GetFirstChild().AppendChild(content);
			//document.GetFirstChild().AppendChild(tooltip);
		}
		tooltip.Show(DocumentFocusFlags::NONE);
		
		string mouse_x = event.GetParameter(Rocket::String("mouse_x"), Rocket::String());
		string mouse_y = event.GetParameter(Rocket::String("mouse_y"), Rocket::String());
		
		content.SetAttribute(Rocket::String("style"),
			Rocket::String("display: block; width: 60px; position: absolute; left: " + mouse_x + "; top: " + mouse_y + ";"));
		
		content.SetInnerRML(Rocket::String('<img src="' + currentTable + '/' + filename + '" width="60px" height="60px"/>'));
	}
}

void MovePreviewPopup(Rocket::Event@ event)
{
	string mouse_x = event.GetParameter(Rocket::String("mouse_x"), Rocket::String());
	string mouse_y = event.GetParameter(Rocket::String("mouse_y"), Rocket::String());
	content.SetAttribute(Rocket::String("style"),
		Rocket::String("display: block; width: 60px; position: absolute; left: " + mouse_x + "; top: " + mouse_y + ";"));
}

void HidePreviewPopup(Rocket::Event@ event)
{
	Rocket::ElementDocument@ document = event.GetTargetElement().GetOwnerDocument();
	if (document !is null)
	{
		if (tooltip !is null)
		{
			content.SetAttribute(Rocket::String("style"), Rocket::String("display: none;"));
			tooltip.Hide();
		}
	}
}
