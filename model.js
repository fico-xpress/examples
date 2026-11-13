// FICO Xpress Examples Browser — implementation of data model
// (C) Copyright 2026 Fair Isaac Corporation

'use strict';

// Map file extensions to Prism language identifiers
const LANG_MAP = {
    mos: 'mosel', mosm: 'mosm',
    c: 'c', 'c.w32': 'c', 'c.unx': 'c',
    cpp: 'cpp', cxx: 'cpp',
    java: 'java',
    py: 'python',
    js: 'javascript',
    cs: 'csharp',
    vb: 'visual-basic', bas: 'visual-basic',
    xml: 'xml', vdl: 'xml', csproj: 'xml', vbproj: 'xml', resx: 'xml',
    html: 'html',
    json: 'json',
    r: 'r', R: 'r',
    m: 'matlab',
    for: 'fortran',
    ipynb: 'json',
};

// Data model for the page
const model = {
    // Text of load error
    error: ko.observable(),

    // Array of Section objects
    sections: ko.observableArray(),

    // Routing configuration - page is always an object with fields 'id' and 'fileId'
    page: ko.observable({
        id: null, fileId: null
    }),
    node: ko.pureComputed(() => {
        if (!model.page().id) return null;
        return model.cache()[model.page().id];
    }),
    pageType: ko.pureComputed(() => {
        // Page type is either 'contents' or 'example'
        if (model.node()?.type==='example') return 'example';
        return 'contents';
    }),

    // Current search configuration
    search: ko.observable(''),
    // Last search configuration
    searchTerm: ko.observable(),
    // searchResults is array of Example
    searchResults: ko.observable(),
}

// 'Cache' is a map from ID to Section/SubSection/Example or File
model.cache = ko.computed(()=>{
    const cache = {};
    model.sections().forEach(section => {
        cache[section.id] = section;
        section.subsections.forEach(subsection => {
            cache[subsection.id] = subsection;
            subsection.examples.forEach(example => {
                cache[example.id] = example;
                example.srcfiles.forEach(file => cache[file.id] = file);
                example.datafiles.forEach(file => cache[file.id] = file);
                example.zipfiles.forEach(file => cache[file.id] = file);
            });
        });
    });
    return cache;
});

// Update page title
model.node.subscribe(newNode => {
    if (newNode && newNode.type==='example') {
        document.title = 'FICO Xpress Examples: ' + newNode.title;
    }
    else {
        document.title = 'FICO Xpress Optimization Examples Repository';
    }
});

// On opening an example, load selected file (or first file)
model.page.subscribe(newPage => {
    const newNode = model.cache()[newPage.id];

    // If newNode is a file, redirect to the example containing the page - this preserves legacy ?id=<fileid> URLs
    if (newNode && newNode.type.endsWith('file')) {
        updateLocation(newNode.parent.id, newNode.id, true);
        model.page({
            id: newNode.parent.id,
            fileId: newNode.id
        })
        return;
    }

    if (newNode && newNode.type==='example' && (newPage.fileId || !newNode.selectedFile()) && newNode.viewableFiles.length>0) {
        let chosenFile = newNode.viewableFiles[0];
        if (newPage.fileId) {
            let possibleFiles = newNode.viewableFiles.filter(f => f.id === newPage.fileId);
            if (possibleFiles.length > 0) {
                chosenFile = possibleFiles[0];
            }
        }
        if (chosenFile) {
            chosenFile.selectFile().then(() => {
                if (newPage.fileId) {
                    // After selected file content is loaded, scroll to show file if we'd asked for a
                    // specific one. This is does as a KnockOut task to ensure we wait until the DOM elements
                    // have been updated with the new file content.
                    ko.tasks.schedule(() => {
                        const a2 = document.getElementById('filedisploc');
                        if (a2) a2.scrollIntoView({ behavior: 'smooth', block: 'start' });
                    });
                }
            });
        }
    }
});

// Ensure parents of selected tree node are expanded
model.node.subscribe(newNode => {
    if (newNode && newNode.type==='example') {
        newNode.parent.expanded(true);
        newNode.parent.parent.expanded(true);
    }
    // On opening a subsection, expand it and its parent
    if (newNode && newNode.type==='subsection') {
        newNode.expanded(true);
        newNode.parent.expanded(true);
    }
    // On opening a section, expand it
    if (newNode && newNode.type==='section') {
        newNode.expanded(true);
    }
})

// On search
model.onClickSearch = () => {
    const keyword = model.search().trim();
    model.searchTerm(keyword);
    if (!keyword) {
        model.searchResults(null);
        return;
    }

    const tokens = keyword.toLowerCase().split(/\s+/).filter(Boolean);
    const results = [];

    function scoreText(t) {
        if (!t) return 0;
        const lower = t.toLowerCase();
        return tokens.filter(tok => lower.includes(tok)).length;
    }

    Object.values(model.cache()).forEach(node => {
        if (node.type !== 'example') return;
        const score = scoreText(node.title)
            + scoreText(node.description)
            + scoreText(node.features)
            + scoreText(node.searchmeta)
            + scoreText(node.parent.searchmeta)
            + scoreText(node.parent.searchcat);
        if (score > 0) results.push({ node, score });
    });

    results.sort((a, b) => b.score - a.score);
    model.searchResults(results.map(r => r.node));

};

/**
 * Return to homepage (no default section/subsection)
 */
model.goHome = () => {
    model.goHomeWithHash(null);
};

/**
 * Return to homepage  with given section/subsection hash (or null)
 */
model.goHomeWithHash = (hash_id) => {
    let url = new URL(window.location);
    url.search = '';
    url.hash = hash_id || '';
    history.pushState({ id: null }, '', url.toString());
    model.page({id: null, fileId: null});
    model.search('');
    model.searchResults('');
};

// ─── Data representation ─────────────────────────────────────────────────────

class Section {
    type = 'section';

    /**
     * Construct Section from a DOM element
     * @param secEl
     */
    constructor(secEl) {
        this.id = secEl.getAttribute('id');
        this.title = textOf(secEl, 'title');
        this.description = textOf(secEl, 'description');
        this.subsections = childrenByTag(secEl, 'subsection').map(subEl => new SubSection(subEl, this));
        this.expanded = ko.observable(false);
    }

    get href() {
        return '#' + encodeURIComponent(this.id);
    }

    toggleExpanded() {
        this.expanded(!this.expanded());
    }
}

class SubSection {
    type = 'subsection';

    /**
     * Construct SubSection from a DOM element
     */
    constructor(subEl, section) {
        this.id = subEl.getAttribute('id');
        this.title = textOf(subEl, 'title');
        this.description = textOf(subEl, 'description');
        this.searchcat = textOf(subEl, 'searchcat');
        this.searchmeta = textOf(subEl, 'searchmeta');
        this.parent = section;
        this.expanded = ko.observable(false);

        // <examples> wrapper element is optional in some subsections
        const examplesContainers = childrenByTag(subEl, 'examples');
        const exampleEls = examplesContainers.length > 0
            ? childrenByTag(examplesContainers[0], 'example')
            : childrenByTag(subEl, 'example');

        this.examples = exampleEls.map(exEl => new Example(exEl, this));
        // Fill in the 'next' and 'prev' link for each example
        this.examples.forEach((ex, i, arr) => {
            if (i > 0) ex.prev = arr[i - 1];
            if (i < arr.length - 1) ex.next = arr[i + 1];
        })
    }

    get href() {
        return '#' + encodeURIComponent(this.id);
    }

    toggleExpanded() {
        this.expanded(!this.expanded());
    }
}

class Example {
    type = 'example';

    /**
     * Construct Example from a DOM element
     */
    constructor(exEl, subsection) {
        const viewableAttr = exEl.getAttribute('viewable');
        const descEl = childrenByTag(exEl, 'description')[0];
        this.id = exEl.getAttribute('id');
        this.title = textOf(exEl, 'title');
        this.description = descEl ? innerXML(descEl) : '';
        this.features = textOf(exEl, 'features');
        this.searchmeta = textOf(exEl, 'searchmeta');
        this.viewable = viewableAttr !== 'false';
        this.parent = subsection;
        // 'prev' and 'next' will be filled in later, with pointers to the previous / next examples
        this.prev = null;
        this.next = null;
        this.srcfiles = childrenByTag(exEl, 'srcfile').map(fEl => new File('srcfile', fEl, this));
        this.datafiles = childrenByTag(exEl, 'datafile').map(fEl => new File('datafile', fEl, this));
        this.zipfiles = childrenByTag(exEl, 'zipfile').map(fEl => new File('zipfile', fEl, this));
        this.isPackaging = ko.observable(false);

        // Currently selected file - this will be populated when File.goFile is called
        this.selectedFile = ko.observable();
    }

    get href() {
        return '?id=' + encodeURIComponent(this.id);
    }

    get allFiles() {
        return [...this.srcfiles, ...this.datafiles];
    }

    get viewableFiles() {
        return this.allFiles.filter(f => f.viewable);
    }

    /**
     * Open this example
     */
    goExample() {
        navigate(this.id, null);
    }

    /**
     * Download all files from this example
     */
    async downloadAllFiles() {
        if (!this.allFiles.length) return;

        this.isPackaging(true);

        try {
            const zip = new JSZip();
            await Promise.all(this.allFiles.map(async f => {
                try {
                    const resp = await fetch(f.srcPath);
                    if (resp.ok) {
                        zip.file(f.srcPath.split('/').pop(), await resp.arrayBuffer());
                    }
                } catch (_) {}
            }));

            const blob = await zip.generateAsync({ type: 'blob' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = this.id + '.zip';
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            setTimeout(() => URL.revokeObjectURL(url), 1000);
        } catch (err) {
            console.error('ZIP creation failed:', err);
        }

        this.isPackaging(false);
    }
}

class File {
    /**
     * Construct a File from a DOM element
     */
    constructor(type, el, parent) {
        const viewableAttr = el.getAttribute('viewable');
        this.type = type;
        this.id = el.getAttribute('id');
        this.path = el.getAttribute('path');
        this.title = el.textContent.trim();
        this.viewable = (type === 'zipfile' ? false : viewableAttr !== 'false');
        this.parent = parent;
        this.content = ko.observable();  // Plain text content; populated on demand
        this.highlightedContent = ko.pureComputed(() => {
            const ext = (this.path.match(/\.([^./]+)$/) || [])[1] || '';
            const lang = LANG_MAP[ext] || LANG_MAP[ext.toLowerCase()] || '';
            if (lang && Prism.languages[lang]) {
                return Prism.highlight(this.content(), Prism.languages[lang], lang);
            } else {
                return escapeHTML(this.content());
            }

        });
        this.loadError = ko.observable();
    }

    // Build a URL for a file referenced in contents.xml.
    // Old-format paths start with '/' (e.g. /bcl/C/file.c → src/bcl/C/file.c).
    // New-format paths are relative (e.g. mosel/Intro/file.mos → src/mosel/Intro/file.mos).
    get srcPath() {
        return 'src/' + this.path.replace(/^\//, '');
    }

    get href() {
        return this.parent.href + "#" + encodeURIComponent(this.id);
    }

    /**
     * Open this file
     */
    goFile() {
        navigate(this.parent.id, this.id);
    }

    /**
     * Open file preview in the browser
     */
    async selectFile() {
        // Set selected file
        this.parent.selectedFile(this);
        // If file content has not been fetched, fetch it
        if (!this.content()) {
            this.loadError(null);
            try {
                const resp = await fetch(this.srcPath);
                if (!resp.ok) throw new Error('HTTP ' + resp.status);
                this.content(await resp.text());
            } catch (e) {
                this.loadError(e.message || e.toString());
            }
        }

    }
}

// ─── Data loading ────────────────────────────────────────────────────────────

async function loadContents() {
    const resp = await fetch('src/contents.xml');
    if (!resp.ok) throw new Error('Failed to fetch src/contents.xml: ' + resp.status);
    const xmlText = await resp.text();
    const xml = new DOMParser().parseFromString(xmlText, 'text/xml');
    const err = xml.querySelector('parsererror');
    if (err) throw new Error('XML parse error: ' + err.textContent);
    return buildModel(xml);
}

// Safe direct-child lookup — avoids :scope which is unreliable on XML DOMs
function childrenByTag(el, tag) {
    return Array.from(el.children).filter(c => c.tagName === tag);
}

function textOf(el, tag) {
    const ch = childrenByTag(el, tag);
    return ch.length > 0 ? ch[0].textContent.trim() : '';
}

// Return inner HTML of an element; falls back to textContent for XML DOMs
// that don't expose innerHTML
function innerXML(el) {
    if (!el) return '';
    if (typeof el.innerHTML === 'string') return el.innerHTML.trim();
    try {
        const s = new XMLSerializer();
        return Array.from(el.childNodes)
            .map(n => s.serializeToString(n))
            .join('')
            .trim();
    } catch (_) {
        return el.textContent.trim();
    }
}

function buildModel(xml) {
    return childrenByTag(xml.documentElement, 'section').map(el => new Section(el));
}

// ─── Router ──────────────────────────────────────────────────────────────────

function navigate(id, fileId) {
    updateLocation(id, fileId);
    route();
}

function updateLocation(id, fileId, replace) {
    let url = window.location.pathname;
    if (id) {
        url = '?id=' + encodeURIComponent(id);
        if (fileId) url += '#' + encodeURIComponent(fileId);
    }
    if (replace) {
        history.replaceState({ id, fileId }, '', url);
    }
    else {
        history.pushState({ id, fileId }, '', url);
    }
}

function parseLocation() {
    const params = new URLSearchParams(window.location.search);
    const hash = window.location.hash ? decodeURIComponent(window.location.hash.substring(1)) : null;
    return {
        // Allow #<id> if no ?id= specified
        id: params.get('id') || hash,
        // File id is carried in the hash (?id=pageid#fileid); fall back to the
        // legacy ?file= query param so previously shared/bookmarked links still work.
        fileId: hash || params.get('file')
    };
}

function route() {
    model.page(parseLocation());
}

/**
 * Route in response to a browser history event. One back/forward step that changes the
 * fragment fires both 'popstate' and 'hashchange', so ignore the second of the pair -
 * re-writing model.page would re-fetch and re-highlight the selected file.
 */
function routeFromHistory() {
    const target = parseLocation();
    const current = model.page();
    if (current.id === target.id && current.fileId === target.fileId) return;
    model.page(target);
}


// ─── Utilities ────────────────────────────────────────────────────────────────

function escapeHTML(s) {
    if (!s) return '';
    return String(s)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}

function buttonover(button)
{
    button.className="button_over";
}
function buttonout(button)
{
    button.className="button";
}

// ─── Events  ────────────────────────────────────────────────────────────────

window.addEventListener('popstate', routeFromHistory);

window.addEventListener('hashchange', routeFromHistory);

document.addEventListener('DOMContentLoaded', async () => {
    try {
        ko.applyBindings(model);
        model.sections(await loadContents());
        route();
    } catch (e) {
        model.error(e.message || e.toString());
    }
});

