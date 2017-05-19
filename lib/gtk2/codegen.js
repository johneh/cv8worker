// Usage:
//   ./jsi.sh -f ../lib/gtk2/codegen.js gtkwidget.so GtkWidget > _gtkwidget.js

if (module.__args.length != 2)
    throw new Error('missing SONAME or GOBJNAME');

const SONAME = __dirname + '/libs/' + module.__args[0];
const GOBJNAME = module.__args[1]; // <- GtkWidget

const funcs = require(SONAME, '-dll').identifiers['#funcs'];

const VOID = 1;
const INT = 2;
const UINT = 3;
const LONG = 4;
const ULONG = 5;
const DOUBLE = 6;
const STRING = 7;
const VOIDPTR = 8;
const RECORDPTR = 9;
const TYPEDEFPTR = 10;
const PTR = 11;

const rejectTypes = [ 'GValue', 'GList', 'GParamSpec' ];

const seenTypes = new Set();
const skipped = []; // static methods
const ignored = []; // commented out

$print(`
const gtklib = require('${SONAME}');
const { GTYPES, register_gtype, gtype_to_js } = require('../gtkb.js');
const OVERRIDES = {};
`);

const incfile = module.__args[0].replace(/\.so$/, ".i");
try {
    $print(readFile(incfile));
} catch(e) {

}

$print(`
const GPROTO = GTYPES['${GOBJNAME}'].proto;
`);

/*
$print(`
const gtklib = require('${SONAME}');
const { GTYPES, register_gtype, gtype_to_js } = require('../gtkb.js');
const GPROTO = GTYPES['${GOBJNAME}'].proto;
`);
*/

funcs.forEach(function(def) {
    if (def.name.startsWith('_'))
        return;

    const argc = def.params.length;

    if (!(argc >= 2 && def.params[1].type === TYPEDEFPTR
            && def.params[1].tagname === GOBJNAME)) {
        skipped.push(def.name);
        return;
    }

//    if (IGNORELIST.has(def.name))
//        return;

    let ignore = false;

    let argstr = '';
    let str = '';
    for (let i = 2; i < argc; i++) {
            let p = def.params[i];
            let pname = p.vname;
            if (argstr !== '') argstr += ',';
            argstr += pname;
            switch (p.type) {
            case STRING:
                str += `  if (typeof ${pname} !== 'string')
    throw new TypeError('${pname} is not a string');
`;
                break;
            case PTR:
            case VOIDPTR:
            case RECORDPTR:
                ignore = true;
                break;
            case TYPEDEFPTR:
                if (rejectTypes.includes(p.tagname)) {
                    ignore = true;
                }
                if(!seenTypes.has(p.tagname)) seenTypes.add(p.tagname);
                str += `  if (!GTYPES['${p.tagname}'].proto.isPrototypeOf(${pname}))
    throw new TypeError('${pname} is not a ${p.tagname}');
`;
                break;
            //case lib.LONG:
            //case lib.ULONG:
            default:
                str += `  if (typeof ${pname} !== 'number' && typeof ${pname} !== 'boolean')
    throw new TypeError('${pname} is not a number');
`;
                if (p.type === INT||p.type === UINT)
                    str += `  ${pname}|= 0;
`;
            }
    }

    const p0 = def.params[0];
    if (p0.type != VOID)
        str += '  const $r = ';
    else
        str += '  ';
    if (argc === 2) {
        str += 'gtklib.' + def.name + '(this);';
    } else {
        str += 'gtklib.' + def.name + '(this,' + argstr + ');';
    }

    // return value
    switch(p0.type) {
        case VOID:
            break;
        case STRING:
            str += `
  if ($r === $nullptr) return null;
  return $utf8String($r, -1);
`;
            break;
        case PTR:
        case VOIDPTR:
        case RECORDPTR:
            ignore = true;
            break;
        case TYPEDEFPTR:
            if (rejectTypes.includes(p0.tagname)) {
                ignore = true;
            }
            if(!seenTypes.has(p0.tagname)) seenTypes.add(p0.tagname);

            // ptr maybe null: GtKWidget *gtk_button_get_image(..)
            str += `
  if ($r === $nullptr) return null;
  return gtype_to_js('${p0.tagname}', 0, $r);`;
            break;
        default:    /* number */
            str += '\n  return $r;';
    }

    if (ignore) {
        $print('/*');
        ignored.push(def.name);
    }

    $print(`GPROTO.${def.name} = function(${argstr}){`);

    $print(str);
    $print('};\n');
    if(ignore) $print('*/');
   
});

$print('Object.assign(GPROTO, OVERRIDES);');
$print('/*== missing static methods ==');
for(let v of skipped) {
    $print(v);
}
$print('*/');
$print('/*== commented out methods ==');
for(let v of ignored) {
    $print(v);
}
$print('*/');
$print('/*== types ==');
for(let v of seenTypes) {
    $print(v);
}
$print('*/');
