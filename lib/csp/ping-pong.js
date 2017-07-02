const Channel = require('../lib/csp/async-channel');

async function sleep(duration) {
  return new Promise(resolve => setTimeout(resolve, duration))
}

async function player(name, table) {
  while (true) {
    let ball = await table.take();
    if (ball === null) { // closed channel 
      console.log(`${name}: table's gone!`);
      break;
    }
    ball.hits++;
    console.log(`${name}! Hits: ${ball.hits}`);
    await sleep(100);
    try {
      await table.put(ball);
    } catch (e) {
      break;
    }
  }
}

async function pingPong() {
  console.log('Opening ping-pong channel!');
  let table = new Channel();
  player('ping', table);
  player('pong', table);
  console.log('Serving ball...');
  let ball = {hits: 0};
  await table.put(ball);
  await sleep(1000);
  console.log('Closing ping-pong channel...');
  table.close();

//    await table.done();
//    console.log('Channel is fully closed!');
//    console.log(`Ball was hit ${ball.hits} times!`);
}

pingPong();

