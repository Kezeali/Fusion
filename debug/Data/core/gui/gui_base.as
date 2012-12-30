void OnWindowLoad(Event@ event)
{
	Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(Rocket::String("title")).SetInnerRML(doc.GetTitle());
}

void HideThisWindow(Event@ ev)
{
	Rocket::ElementDocument @doc = ev.GetCurrentElement().GetOwnerDocument();
	doc.Hide();
}

void UnloadThisWindow(Event@ ev)
{
	Rocket::ElementDocument @doc = ev.GetCurrentElement().GetOwnerDocument();
	doc.Close();
	//doc.GetContext().UnloadDocument(doc);
}

void CloseThisWindow(Event@ ev)
{
	ev.GetCurrentElement().GetOwnerDocument().DispatchEvent(Rocket::String("close"));
}
