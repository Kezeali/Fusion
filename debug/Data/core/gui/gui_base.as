void OnWindowLoad(Event@ event)
{
	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(rString("title")).SetInnerRML(doc.GetTitle());
}

void HideThisWindow(Event@ ev)
{
	ElementDocument @doc = ev.GetCurrentElement().GetOwnerDocument();
	doc.Hide();
}

void UnloadThisWindow(Event@ ev)
{
	ElementDocument @doc = ev.GetCurrentElement().GetOwnerDocument();
	doc.GetContext().UnloadDocument(doc);
}

void CloseThisWindow(Event@ ev)
{
	ev.GetCurrentElement().GetOwnerDocument().DispatchEvent(rString("close"));
}
