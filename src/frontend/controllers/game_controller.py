class GameController:
    pass
#     def upload_script(self, filepath: str):
#         """Upload game script to server"""
#         with open(filepath, "r", encoding="utf-8") as f:
#             script_content = f.read()

#         msg = Message(
#             type=MessageType.UPLOAD_SCRIPT,
#             payload={
#                 "format": "simple",
#                 "content": script_content,
#                 "mode": "auto",  # Server auto-plays both sides
#             },
#         )
#         self.session.send_message(msg)

#         # Switch to watching state
#         self.state = WatchingState()
#         logger.info("Script uploaded. Watching game execution...")
