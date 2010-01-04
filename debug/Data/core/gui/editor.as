//class EditorDataSource : IDataSource
//{
//	EditorDataSource()
//	{
//	}
//	~EditorDataSource()
//	{
//	}
//
//	StringArray _suggestions;
//
//	void UpdateSuggestions(const string &in search)
//	{
//		editor.searchTypes(_suggestions, search);
//	}
//
//	void GetRow(StringList&out row, const e_String&in table, int row_index, const StringList&in columns)
//	{
//		if (row_index >= _suggestions.size())
//			return;
//
//		if (table == e_String("type_suggestions"))
//		{
//			for (uint i = 0; i < columns.size(); i++)
//			{
//				if (columns[i] == e_String("name"))
//				{
//					row.push_back(e_String(_suggestions[row_index]));
//				}
//			}
//		}
//	}
//
//	int GetNumRows(const e_String&in table)
//	{
//		if (table == e_String("type_suggestions"))
//		{
//			return _suggestions.size();
//		}
//
//		return 0;
//	}
//}

//EditorDataSource@ ed = EditorDataSource();

void OnEntityTypeChanged(Event@ event)
{
	string type = event.GetParameter(e_String('value'), e_String());
	editor.updateSuggestions(type);
	editor.setEntityType(type);
}

void OnClickPseudoCheckbox(Event@ event)
{
	bool mode = event.GetParameter(e_String('checked'), false);
	editor.setEntityMode(mode);
}

void OnSelected(Event@ event)
{
	int index = event.GetParameter(e_String("row_index"), -1);
	if (index == -1)
		return;

	string type = editor.getSuggestion(index);

	FormControlInput@ element = cast<FormControlInput>( module_document.GetElementById(e_String("entity_type")) );
	if (element !is null)
	{
		element.SetValue(e_String(type));
		editor.setEntityType(type);
	}
}

void SubmitSaveOrLoad(Event@ event)
{
	string filename = event.GetParameter(e_String('filename'), e_String());
	string type = event.GetParameter(e_String('submit'), e_String());
	if (type == "load")
		editor.load( filename );
	else if (type == "save")
		editor.save( filename );
	else
		editor.compile( filename );
}
