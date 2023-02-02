import asyncio
from command_driver import CommandDriver

async def main():
    try:
        client = CommandDriver(asyncio.get_event_loop())
        await client.start()
        # client.loop.run_until_complete(client.cmdloop())
    except Exception as e:
        print('Error in main loop: {}'.format(e))
    finally:
        client.loop.close()

if __name__ == "__main__":
    asyncio.run(main())
