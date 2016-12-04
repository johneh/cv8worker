(function () {
    var load = function (loader, argv) {
        // this === global object in main
        //      === parent module, otherwise.

        var argc = argv.length;
        var isFirst = false;
        var modulePath;
        var i;

        if (! loader.hasOwnProperty('__main__')) {
            isFirst = true;
            loader._cwd = null;
            loader._moduleCache = Object.create(null);
            modulePath = loader.path = loader.path||'';
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
                let dp = $malloc(8);
                dp.pack(0, 'j', delay);
                $go(loader.msleep, dp).then(function(data) {
                        dp.free();
                        callback();
                    });
            };

            // unique Id (for use as ctypeid etc.) 
            loader._uId = 0;
            this.$uId = function () {
                return ++loader._uId;
            };
            this.console = { log : $print };
        } else
            modulePath = this.__path;	// this === parent module

        // $print("modulePath =", modulePath);

        var path, filename;

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

        var cache = loader._moduleCache;
        if (({}).hasOwnProperty.call(cache, filename)) {
            return cache[filename].exports;
        }

        var module = {};
        module.__filename = filename;

        // XXX: starts with this default, can be changed in each module.
        module.__path = loader._defaultPath;
        var dirname = module.__dirname
                = filename.substring(0, filename.lastIndexOf('/'));
        var exports = module.exports = {};

        if (isFirst)
            loader.__main__ = module;
        module.main = loader.__main__;

        var moduleSource = loader.readFile(filename);
        if (moduleSource === null) {
            throw new Error(path + ': error reading file'); // FIXME strerror(errno)
        }

        moduleSource = "(function (exports, require, module, __filename, __dirname){"
                        + moduleSource + "\n})";

        cache[filename] = module;
        var moduleWrap = $eval(moduleSource, path);
        var lastDir = loader._cwd;
        loader._cwd = dirname;
        var require = function (path) {
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

    var _findFile = function (path, loader) {
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

    var findFile = function (path, loader, searchPath) {
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

    var dlloader = function () {
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

        dll.prototype.sizeof = function(s) {
            s = s + '';
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
            // throw ?
        };

        dll.prototype.offsetof = function (s, item) {
            s = s + '';
            let t = null;
            if (this._types.hasOwnProperty(s)) {
                t = this._types[s];
            } else if (this._tags.hasOwnProperty(s)) {
                t = this._tags[s];
            }
            if (t !== null) {
                if (!item)
                    return JSON.stringify(t, function (k, v) {
                            if (k != '#size') return v;
                        });
                item = item + '';
                if (item != '#size' && t.hasOwnProperty(item))
                    return t[item];
            }
            // throw ?
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

