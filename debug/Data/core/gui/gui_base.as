void OnWindowLoad(Rocket::Event@ event)
{
	Rocket::ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(Rocket::String("title")).SetInnerRML(doc.GetTitle());
}

void HideThisWindow(Rocket::Event@ ev)
{
	Rocket::ElementDocument @doc = ev.GetCurrentElement().GetOwnerDocument();
	doc.Hide();
}

void UnloadThisWindow(Rocket::Event@ ev)
{
	Rocket::ElementDocument @doc = ev.GetCurrentElement().GetOwnerDocument();
	doc.Close();
	//doc.GetContext().UnloadDocument(doc);
}

void CloseThisWindow(Rocket::Event@ ev)
{
	ev.GetCurrentElement().GetOwnerDocument().DispatchEvent(Rocket::String("close"));
}
