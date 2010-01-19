void OnWindowLoad(Event@ event)
{
	Document @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(e_String("title")).SetInnerRML(doc.GetTitle());
}
