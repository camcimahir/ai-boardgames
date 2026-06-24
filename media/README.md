# Media assets

Drop your screenshots and GIFs here. The main `readme.md` already references these file names,
so once you add them they'll show up automatically:

| File | Used for | Suggested capture |
| --- | --- | --- |
| `chess.gif` | Hero animation at the top of the README | A short clip of a game vs the AI (capture move + AI reply) |
| `chess.png` | Chess section still | A clean mid-game board |
| `connect-four.png` | Connect Four section | A board with a few discs / a win |
| `tictactoe.png` | Tic-Tac-Toe section | A finished or in-progress grid |
| `astrobots.gif` | AstroBots section | A few seconds of ships fighting |

Tips for good GIFs:
- Keep them short (3–8s) and under ~10 MB so the README loads fast on GitHub.
- Tools: [ScreenToGif](https://www.screentogif.com/) (Windows), or record an `.mp4` and convert with ffmpeg:
  `ffmpeg -i clip.mp4 -vf "fps=15,scale=720:-1:flags=lanczos" chess.gif`
