void OnWindowLoad(Event@ event)
{
	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(e_String("title")).SetInnerRML(doc.GetTitle());
}

void HideThisWindow(Event@ ev)
{
	ElementDocument @doc = ev.GetCurrentElement().GetOwnerDocument();
	doc.Hide();
}

void CloseThisWindow(Event@ ev)
{
	ev.GetCurrentElement().GetOwnerDocument().DispatchEvent(e_String("close"));
}
