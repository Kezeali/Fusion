void OnEditorWindowLoad(Event@ event)
{
	Document @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(e_String("title")).SetInnerRML(doc.GetTitle());

	ElementUndoMenu @undoMenu = cast<ElementUndoMenu>( doc.GetElementById(e_String("undo_menu")) );
	if (undoMenu !is null)
		editor.attachUndoMenu(undoMenu);

	@undoMenu = doc.GetElementById(e_String("redo_menu"));
	if (undoMenu !is null)
		editor.attachUndoMenu(undoMenu);
}

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
