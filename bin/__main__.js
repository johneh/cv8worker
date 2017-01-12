(function () {
    const load = function (loader, argv) {
        // this === global object in main
        //      === parent module, otherwise.

        let argc = argv.length;
        let isFirst = false;
        let modulePath;
        let i;

        if (! loader.hasOwnProperty('__main__')) {
            isFirst = true;
            loader._cwd = null;
            loader._moduleCache = Object.create(null);
            if (typeof loader.path !== 'string')
                loader.path = '';
            modulePath = loader.path;
            if ((i = argv.indexOf('-p')) >= 0 && argc > i && argv[i+1]) {
                modulePath = argv[i+1];
                if (loader.path)
                    modulePath += (':' + loader.path);
            }
            loader._defaultPath = modulePath;

            this.DLL = dlloader();
            this.$loadlib = function (libname) {
                return new DLL(libname, loader._cwd).identifiers;
            };

            this.setTimeout = function (callback, delay) {
                if (typeof callback !== 'function') {
                    throw new TypeError("setTimeout: 'callback' is not a function");
                }
                delay |= 0;
                if (delay < 0)
                    delay = 0;
                $co(loader.msleep, delay)
                .then(() => {
                    callback();
                });
            };

            this.$fdevent = loader.fdevent;

            this.console = { log: $print };

            this.$$onPromiseReject(function(event, promise, err) {
                // event -- v8.h
                //  kPromiseRejectWithNoHandler = 0,
                //  kPromiseHandlerAddedAfterReject = 1
                if (event === 0)
                    console.log('Unhandled rejection:', promise, err.stack);
                else
                    console.log('Post-rejection handler:', promise, err.stack);
            });

        } else
            modulePath = this.__path;	// this === parent module

        // $print("modulePath =", modulePath);

        let path, filename;

        if (argc == 0) {
            throw new Error('usage: jsi -f filename');
        } else if ((i = argv.indexOf('-e')) >= 0 && argc > i) {
            /* execute string argv[i+1] */
            throw new Error('Not ready yet');
        } else if ((i = argv.indexOf('-f')) >= 0 && argc > i) {
            path = argv[i+1];
        } else {
            path = argv[0];
        }

        filename = findFile(path, loader, modulePath);
        if (!filename)
            throw new Error(path + ': No such file');
        //  $print("filename = ", filename);

        const cache = loader._moduleCache;
        if (({}).hasOwnProperty.call(cache, filename)) {
            return cache[filename].exports;
        }

        const module = {};
        module.__filename = filename;

        // XXX: starts with this default, can be changed in each module.
        module.__path = loader._defaultPath;
        const dirname = module.__dirname
                = filename.substring(0, filename.lastIndexOf('/'));
        const exports = module.exports = {};

        if (isFirst)
            loader.__main__ = module;
        module.main = loader.__main__;

        let moduleSource = loader.readFile(filename);
        if (moduleSource === null) {
            throw new Error(path + ': error reading file'); // FIXME strerror(errno)
        }

        moduleSource = "(function (exports, require, module, __filename, __dirname){"
                        + moduleSource + "\n})";

        cache[filename] = module;
        const moduleWrap = $eval(moduleSource, path);
        const lastDir = loader._cwd;
        loader._cwd = dirname;
        const require = function (path) {
            return load.call(module, loader, [ "-f", path ]);
        };
        moduleWrap.call(exports,
                exports,
                require,
                module,
                filename,
                dirname
        );
        loader._cwd = lastDir;

        return module.exports;
    }; // load

    const _findFile = function (path, loader) {
        if (/^\.\.?\//.test(path) && loader._cwd)
            path = loader._cwd + '/' + path;
        let found = loader.isRegularFile(path);
        if (! found && path.lastIndexOf('.js') < 0) {
            path += '.js';
            found = loader.isRegularFile(path);
        }
        if (! found)
            return null;
        return loader.realPath(path);
    };

    const findFile = function (path, loader, searchPath) {
        let filename = _findFile(path, loader);
        if (filename !== null)
            return filename;
        if (/^\.\.?\//.test(path))
            return null;
        let pathList = searchPath.split(':');
        let tryDir;
        for (let i = 0; i < pathList.length; i++) {
            tryDir = pathList[i].trim();
            if (tryDir != '') {
                filename = _findFile(tryDir + '/' + path, loader);
                if (filename !== null)
                    return filename;
            }
        }
        return null;
    };

    const dlloader = function () {
        let dll = function (libname, dirname) {
            libname = libname + '';
            if (/^\.\.?\//.test(libname) && dirname)
                libname = dirname + '/' + libname;
            let lib;
            try {
                lib = this._lib = $load(libname); // null if path has no slash ???
            } catch (e) {
                lib = null;
            }
            if (lib === null || ! (lib instanceof Object))
                throw new Error('DLL: error loading library \'' + libname + '\'');
            if (lib.hasOwnProperty('#types')) {
                this._types = lib['#types'];
                delete this._lib['#types'];
            } else {
                this._types = {};
            }
            if (lib.hasOwnProperty('#tags')) {
                this._tags = lib['#tags'];
                delete this._lib['#tags'];
            } else {
                this._tags = {};
            }
        };

        const _getRecord = function(lib, recName) {
            // try typedef
            if (lib._types.hasOwnProperty(recName)) {
                return lib._types[recName];
            }
            // try struct
            if (lib._tags.hasOwnProperty(recName)) {
                return lib._tags[recName];
            }
            return null;
        }

        dll.prototype.structDesc = function (name) {
            // if (typeof name !== 'string') ..
            name = name + '';
            const t = _getRecord(this, name);
            if (t === null)
                throw new Error('invalid record name');
            let s = {};
            for (let field in t) {
                if (field === '#size')
                    continue;
                let ct = (t[field] >> 16);
                if (ct)
                    ct = String.fromCharCode(ct);
                else
                    ct = null;
                s[field] = { offset : (t[field] & 0xFFFF), type : ct };
            }
            return s;
        }

        dll.prototype.sizeOf = function(s) {
            if (typeof s !== 'string')
                throw new TypeError('invalid argument');
            // try typedefs
            if (this._types.hasOwnProperty(s)) {
                let t = this._types[s];
                if (t.hasOwnProperty('#size')) return t['#size'];
            }
            // try structs
            if (this._tags.hasOwnProperty(s)) {
                let t = this._tags[s];
                if (t.hasOwnProperty('#size')) return t['#size'];
            }
            throw new Error('non-existent struct');
        };

        dll.prototype.offsetOf = function (s, item) {
            if (typeof s !== 'string' || typeof item !== 'string')
                throw new TypeError('invalid argument(s)');
            let t = _getRecord(this, s);
            if (t !== null && item !== '#size' && t.hasOwnProperty(item))
                return (t[item] & 0xFFFF);
            throw new Error('non-existent record or field');
        };

        dll.prototype.typeOf = function (s, item) {
            if (typeof s !== 'string' || typeof item !== 'string')
                throw new TypeError('invalid argument(s)');
            let t = _getRecord(this, s);
            if (t !== null && item !== '#size' && t.hasOwnProperty(item)) {
                let ct = (t[item] >> 16);
                if (ct)
                    return String.fromCharCode(ct);
                return null;    // struct or union
            }
            throw new Error('non-existent record or field');
        };

        Object.defineProperty(dll.prototype, "identifiers", {
            get: function() {
                    return this._lib;
                }
            }
        );

        return dll;
    };

    return load;
})();

