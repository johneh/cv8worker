const sqlite3 = require('../lib/sqlite3.js');
(async function() {
    let dbh = null;
    try {
        dbh = await sqlite3.open("./testsqlite3.db");
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
        count = 0;
        await sth.each((row) => {
            if (row !== null) {
                console.log(row[0], row[1]);
                count++;
            }
        });

        console.log(`========== each: ${count} rows fetched. ==========`);
        await sth.finalize();

    } catch (err) {
        console.log(err);
    } finally {
        await dbh.close();
        console.log('Success!');
    }
})();

