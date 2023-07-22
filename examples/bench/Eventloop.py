import aiohttp
import aiohttp.web


import asyncio
import unittest
import weakref
import sys
from uvloop import async_timed

class TestAioHTTP:
    def __init__(self, loop):
        self.loop = loop

    def test_aiohttp_basic_1(self):
        PAYLOAD = '<h1>It Works!</h1>' * 10000

        async def on_request(request):
            return aiohttp.web.Response(text=PAYLOAD)

        asyncio.set_event_loop(self.loop)
        app = aiohttp.web.Application()
        app.router.add_get('/', on_request)

        runner = aiohttp.web.AppRunner(app)
        self.loop.run_until_complete(runner.setup())
        site = aiohttp.web.TCPSite(runner, '0.0.0.0', '10000')
        self.loop.run_until_complete(site.start())
        port = site._server.sockets[0].getsockname()[1]

        @async_timed()
        async def test():
            # Make sure we're using the correct event loop.
            assert asyncio.get_event_loop() == self.loop

            for addr in (('localhost', port),
                         ('127.0.0.1', port)):
                async with aiohttp.ClientSession() as client:
                    async with client.get('http://{}:{}'.format(*addr)) as r:
                        assert r.status, 200
                        result = await r.text()
                        assert result, PAYLOAD

        self.loop.run_until_complete(test())
        self.loop.run_until_complete(runner.cleanup())


    def test_aiohttp_graceful_shutdown(self):
        async def websocket_handler(request):
            ws = aiohttp.web.WebSocketResponse()
            await ws.prepare(request)
            request.app['websockets'].add(ws)
            try:
                async for msg in ws:
                    await ws.send_str(msg.data)
            finally:
                request.app['websockets'].discard(ws)
            return ws

        async def on_shutdown(app):
            for ws in set(app['websockets']):
                await ws.close(
                    code=aiohttp.WSCloseCode.GOING_AWAY,
                    message='Server shutdown')

        asyncio.set_event_loop(self.loop)
        app = aiohttp.web.Application()
        app.router.add_get('/', websocket_handler)
        app.on_shutdown.append(on_shutdown)
        app['websockets'] = weakref.WeakSet()

        runner = aiohttp.web.AppRunner(app)
        self.loop.run_until_complete(runner.setup())
        site = aiohttp.web.TCPSite(runner, '0.0.0.0', '10000')
        self.loop.run_until_complete(site.start())
        port = site._server.sockets[0].getsockname()[1]

        async def client():
            async with aiohttp.ClientSession() as client:
                async with client.ws_connect(
                    'http://127.0.0.1:{}'.format(port)) as ws:
                    await ws.send_str("hello")
                    async for msg in ws:
                        assert msg.data == "hello"

        client_task = asyncio.ensure_future(client())

        @async_timed()
        async def stop():
            await asyncio.sleep(0.1)
            try:
                await asyncio.wait_for(runner.cleanup(), timeout=0.1)
            except Exception as e:
                print(e)
            finally:
                try:
                    client_task.cancel()
                    await client_task
                except asyncio.CancelledError:
                    pass

        self.loop.run_until_complete(stop())

import uvloop
class UVLoop:
    implementation = 'uvloop'

    def new_loop(self):
        return uvloop.new_event_loop()

    def new_policy(self):
        return uvloop.EventLoopPolicy

import asyncio
class AIOLoop:
    implementation = 'asyncio'

    def new_loop(self):
        return asyncio.new_event_loop()

    def new_policy(self):
        return asyncio.DefaultEventLoopPolicy


if __name__ == "__main__":

    def main(self) :
        print(f"tesing {self.implementation} eventloop")
        asyncio.DefaultEventLoopPolicy = self.new_policy()

        #TestAioHTTP(self.new_loop()).test_aiohttp_basic_1()
        TestAioHTTP(self.new_loop()).test_aiohttp_graceful_shutdown()

    #main(AIOLoop())
    main(UVLoop())
