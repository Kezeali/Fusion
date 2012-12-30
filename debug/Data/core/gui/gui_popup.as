void GeneratePreviewPopup(string filename)
{
	Rocket::ElementDocument@ popup = gui.getContext('editor').LoadDocument("/Data/core/gui/popup.rml");
	Rocket::Element@ content = popup.GetElementById("content");
	content.SetInnerRML('<img src="' + filename + '"/>');
	popup.Show();
}