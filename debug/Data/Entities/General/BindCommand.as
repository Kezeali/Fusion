// Binds a command with callback and Help text (H)
void CCBind_H(const string &in command, const string &in callback, const string &in help_text)
{
	console.bindCommand(command, callback);
	console.setCommandHelpText(command, help_text);
}

// Binds a command with callback and Argument names (Args)
void CCBind_Args(const string &in command, const string &in callback, const string &in argument_names)
{
	console.bindCommand(command, callback);
	console.setCommandHelpText(command, "", argument_names);
}

// Binds a command with callback, Help text and Argument names (HA)
void CCBind_HA(const string &in command, const string &in callback, const string &in help_text, const string &in argument_names)
{
	console.bindCommand(command, callback);
	console.setCommandHelpText(command, help_text, argument_names);
}

// Binds a command with callback, Auto-correct callback, Help text and Argument names (AHA)
void CCBind_AHA(const string &in command, const string &in callback, const string &in autocorrect, const string &in help_text, const string &in argument_names)
{
	console.bindCommand(command, callback, autocorrect);
	console.setCommandHelpText(command, help_text, argument_names);
}

// Binds a command with callback, Auto-correct callback and Argument names (AA)
void CCBind_AA(const string &in command, const string &in callback, const string &in autocorrect, const string &in argument_names)
{
	console.bindCommand(command, callback, autocorrect);
	console.setCommandHelpText(command, "", argument_names);
}
