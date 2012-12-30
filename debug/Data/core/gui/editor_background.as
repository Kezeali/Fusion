void OnDragDrop(Event@ event)
{
	Rocket::Element@ dragElement = event.GetParameterAsElement(Rocket::String('drag_element'));
	int posx = event.GetParameter("mouse_x", int(0));
	int posy = event.GetParameter("mouse_y", int(0));

	Rocket::ElementList elements;
	dragElement.GetElementsByTagName(elements, Rocket::String("fileinfo"));
	if (elements.size() > 0)
	{
		Rocket::Element@ fileInfo = elements[0];
		string textval = fileInfo.GetAttribute(Rocket::String("path"), Rocket::String());
		Vector worldPosition = editor.getViewport("main").ScreenToWorld(Vector(float(posx), float(posy)));
		editor.createArchetypeInstance(textval, worldPosition, 0.0);
	}
}
