ElementUndoMenu @editor_undoMenu = null;
ElementUndoMenu @editor_redoMenu = null;

void OnUndo(Event@ event)
{
	if (editor_undoMenu is null)
	{
		Document @doc = event.GetCurrentElement().GetOwnerDocument();
		@editor_undoMenu = cast<ElementUndoMenu>( doc.GetElementById(e_String("undo_menu")) );
	}

	editor.undo(editor_undoMenu.GetSelection());
}

void OnRedo(Event@ event)
{
	if (editor_redoMenu is null)
	{
		Document @doc = event.GetCurrentElement().GetOwnerDocument();
		@editor_redoMenu = cast<ElementUndoMenu>( doc.GetElementById(e_String("redo_menu")) );
	}

	editor.redo(editor_redoMenu.GetSelection());
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

	ElementFormControlInput@ element = cast<ElementFormControlInput>( module_document.GetElementById(e_String("entity_type")) );
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