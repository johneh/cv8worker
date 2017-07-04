const sqlite3 = require('../lib/sqlite3.js');
(async function() {
    let dbh = null;
    try {
        dbh = await sqlite3.open("./testsqlite3.db");

        let version = (await dbh.get("select sqlite_version()"))[0];
        console.log('SQLite3 Version:', version);

        let [v] = await dbh.transaction(db => {
            return db.get("select sqlite_version()");
        });
        console.log('SQLite3 Version:', v);

        let sth = await dbh.prepare(
                    "select customerNumber, customerName from customers");
        let count = 0;
        let row;
        while (row = await sth.next()) {
            console.log(row[0], row[1]);
            count++;
        }
        console.log(`========== ${count} rows fetched. ==========`);
        await sth.finalize();

        sth = await dbh.prepare(`
select customerNumber, customerName from customers where customerNumber > ?
                and  customerNumber < ?`
        );

        await sth.bind(400, 425);
        count = await sth.each((row) => {
            console.log(row[0], row[1]);
        });

        console.log(`========== each: ${count} rows fetched. ==========`);

        await sth.bind(400, 425);
        let r = await sth.all();
        for (let i = 0; i < r.length; i++) {
            console.log(r[i][0], r[i][1]);
        }

        console.log(`========== all: ${r.length} rows fetched. ==========`);

        await sth.finalize();

    } catch (err) {
        console.log(err);
    } finally {
        await dbh.close();
        console.log('Success!');
    }
})();

