void OnDragDrop(Event@ event)
{
	Element@ dragElement = event.GetParameterAsElement(rString('drag_element'));
	int posx = event.GetParameter("mouse_x", int(0));
	int posy = event.GetParameter("mouse_y", int(0));

	ElementList elements;
	dragElement.GetElementsByTagName(elements, rString("fileinfo"));
	if (elements.size() > 0)
	{
		Element@ fileInfo = elements[0];
		string textval = fileInfo.GetAttribute(rString("path"), rString());
		Vector worldPosition = editor.getViewport("main").ScreenToWorld(Vector(float(posx), float(posy)));
		editor.createArchetypeInstance(textval, worldPosition, 0.0);
	}
}
