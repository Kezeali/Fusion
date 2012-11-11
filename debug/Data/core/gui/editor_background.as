void OnDragDrop(Event@ event)
{
	Element@ dragElement = event.GetParameterAsElement(rString('drag_element'));

	ElementList elements;
	dragElement.GetElementsByTagName(elements, rString("fileinfo"));
	if (elements.size() > 0)
	{
		Element@ fileInfo = elements[0];
		string textval = fileInfo.GetAttribute(rString("path"), rString());
		editor.createArchetypeInstance(textval, Vector(), 0.0);
	}
}
