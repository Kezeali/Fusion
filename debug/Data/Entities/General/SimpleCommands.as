int correctNumber = 0;

string CC_startGuess(const StringArray &in args)
{
	console.println("Guess a number between 1 and 100");
	correctNumber = rand() * 100 + 1;
	return "";
}

string CC_takeGuess(const StringArray &in args)
{
	if (args.size() >= 2)
	{
		int guess;
		bool validGuess = args[1].parseInt(guess);
		if (validGuess)
		{
			if (guess == correctNumber)
				return "You Win!";
			else if (guess < correctNumber)
				return "Try higher";
			else if (guess > correctNumber)
				return "Try lower";
		}
	}

	return "Guess a number";
}

string CC_echo(const StringArray &in args)
{
	if (args.size() >= 2)
		return args[1];

	return "";
}

void BindSimpleCommands()
{
	CCBind_HA("echo", "CC_echo",
		"Writes whatever you type after it into the console", "[text] [...]");

	CCBind_H("play", "string CC_startGuess(const StringArray &in args)", "Play a guessing game");
	CCBind_HA("guess", "string CC_takeGuess(const StringArray &in args)",
		"Take a guess at the guessing game. Enter the command 'play' first.", "number");
}