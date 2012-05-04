class EditButtonFormatter : IDataFormatter
{
	EditButtonFormatter()
	{
		AddDataFormatter(rString("edit_button"), this);
	}
	~EditButtonFormatter()
	{
	}
	
	rString FormatData(const StringList &in raw_data)
	{
		string formattedData = "";
		if (raw_data.size() >= 2)
		{
			string r0 = raw_data[0];
			string r1 = raw_data[1];
			//formattedData =
			//	"<div class=\"edit_resource\" onclick=\"%this:OnEditResource('"+ r0 +"');\"><img src=\"/core/gui/edit_resource.png\"/></div>";
			formattedData =
				"<div class=\"resource_row\">"
				"<div class=\"r_filename\">" + r1 + "</div>"
				"<fileinfo class=\"r_path\" style=\"display: none;\" path=\"" + r0 + "\"/>";
			if (editor.isResourceEditable(r0))
			{
				formattedData +=
					"<div class=\"edit_resource\" onclick=\"%this:OnEditResource('"+ r0 +"');\"><img src=\"/core/gui/edit_resource.png\"/></div>";
			}
			formattedData += "</div>";
		}
		return formattedData;
	}
};

EditButtonFormatter@ editButtonFormatter = @EditButtonFormatter();

void OnResourceBrowserWindowUnload(Event@ event)
{
	OnWindowUnload(event);
	RemoveDataFormatter(rString("edit_button"));
}

void OnEditResource(string path)
{
	editor.startResourceEditor(path);
}

