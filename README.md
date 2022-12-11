
# ICMPshell
-----------------

The attacker already has root access to the system. So I wrote an install script (run as root) to embed the backdoor into an Ubuntu. The backdoor starts with the system, hence is persistent and accessible whenever the system is started. It throws error messages to systemd. Those can usually be ignored.

## The Protocol

The backdoor sends ICMP echo requests to a hardcoded IP Address (change before use), hence it can only be used by the attacker, and sends "EXPECT CMD: " to show it awaits a command. The server then sends a command back inside the reply.

Next, the backdoor executes the command and sends the stdout in (multiple) packets prefixed by "RES: ". The Server prints everything after "RES: " to the output.

Once the command is complete and all output send, the backdoor goes back to "EXPECT CMD: ". The Server turns the first occurance of it into a prompt and can then reply to a request as soon as the command is entered by the attacker.

The backdoor always waits for the reply until it can be assumed dead. This way race conditions within the protocol should be handled by the Network card by discarding dead packets.

## Potential problems

The request speed is unusually fast and checksums are not calculated. This is not a problem in the IP protocol, but both can be easily detected by a IDS.

The whole process can be Man-in-the-middled so that third parties can gain access to the shell. This could be solved by digitally signing the commands.

## Missing Feature

cd does not work as in a real shell due to the lack of a state inside the backdoor. It could be added in a similar way as to the [HTSH](https://github.com/b3ny4/htsh), or by changing the whole processes cwd, or by spawning an actual shell and use it in general. Curerntly you have to execute commands within another directory by piping the cd command.

The system was only tested within a virtualized network. Packet loss could occur. Some error detection mechanism such as sliding window would improve the stability.

