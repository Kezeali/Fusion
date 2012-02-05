void GeneratePreviewPopup(string filename)
{
	ElementDocument@ popup = gui.getContext('editor').LoadDocument("/core/gui/popup.rml");
	Element@ content = popup.GetElementById("content");
	content.SetInnerRML('<img src="' + filename + '"/>');
	popup.Show();
}