void main()
{
	//...
	FusionScene *scene = new FusionScene();
	FusionNode* child = scene->CreateNode();
	scene->GetRootNode()->AddChild(child);
	//...
}
// OR
void main()
{
	//...
	FusionScene *scene = new FusionScene();
	scene->GetRootNode()->CreateChildNode();
	//...
}