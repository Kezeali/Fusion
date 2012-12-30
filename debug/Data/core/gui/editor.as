Rocket::ElementUndoMenu @editor_undoMenu = null;
Rocket::ElementUndoMenu @editor_redoMenu = null;

void OnEditorWindowLoad(Rocket::Event@ event)
{
	Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(Rocket::String("title")).SetInnerRML(doc.GetTitle());

	@editor_undoMenu = cast<Rocket::ElementUndoMenu>( doc.GetElementById(Rocket::String("undo_menu")) );
	if (editor_undoMenu !is null)
		editor.attachUndoMenu(editor_undoMenu);

	@editor_redoMenu = cast<Rocket::ElementUndoMenu>( doc.GetElementById(Rocket::String("redo_menu")) );
	if (editor_redoMenu !is null)
		editor.attachRedoMenu(editor_redoMenu);
}

void OnUndo(Rocket::Event@ event)
{
	if (editor_undoMenu is null)
		return;
	//{
	//	Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	//	@editor_undoMenu = cast<Rocket::ElementUndoMenu>( doc.GetElementById(Rocket::String("undo_menu")) );
	//}

	editor.undo(editor_undoMenu.GetSelection());
}

void OnRedo(Rocket::Event@ event)
{
	if (editor_redoMenu is null)
		return;
	//{
	//	Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	//	@editor_redoMenu = cast<Rocket::ElementUndoMenu>( doc.GetElementById(Rocket::String("redo_menu")) );
	//}

	editor.redo(editor_redoMenu.GetSelection());
}

void OnClickActualGFXCheckbox(Rocket::Event@ event)
{
	bool enabled = event.GetParameter(Rocket::String("value"), false);
	editor.setDisplayActualSprites(enabled);
}

void OnEntityTypeChanged(Rocket::Event@ event)
{
	string type = event.GetParameter(Rocket::String('value'), Rocket::String());
	editor.updateSuggestions(type);
	editor.setEntityType(type);
}

void OnClickPseudoCheckbox(Rocket::Event@ event)
{
	bool pseudo = event.GetParameter(Rocket::String("value"), false);
	editor.setPlacePseudoEntities(pseudo);
}

void OnSelected(Rocket::Event@ event)
{
	int index = event.GetParameter(Rocket::String("row_index"), -1);
	if (index == -1)
		return;

	string type = editor.getSuggestion(index);

	Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	Rocket::ElementFormControlInput@ element = cast<Rocket::ElementFormControlInput>( doc.GetElementById(Rocket::String("entity_type")) );
	if (element !is null)
	{
		element.SetValue(Rocket::String(type));
		editor.setEntityType(type);
	}
}

void SubmitSaveOrLoad(Rocket::Event@ event)
{
	string filename = event.GetParameter(Rocket::String('filename'), Rocket::String());
	string type = event.GetParameter(Rocket::String('submit'), Rocket::String());
	if (type == "load")
		editor.load( filename );
	else if (type == "save")
		editor.save( filename );
	else
		editor.compile( filename );
}
