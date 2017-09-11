<!DOCTYPE html>
<html lang="en">
<head>
  <meta id="bb-bootstrap" data-current-user="{&quot;isKbdShortcutsEnabled&quot;: true, &quot;isSshEnabled&quot;: false, &quot;isAuthenticated&quot;: false}"
 />
  
  
  <meta http-equiv="X-UA-Compatible" content="IE=edge" />
  <meta charset="utf-8">
  <title>
  phjounin / tftpd64 / wiki / Download Tftpd64 &mdash; Bitbucket
</title>
  <script type="text/javascript">window.NREUM||(NREUM={}),__nr_require=function(e,n,t){function r(t){if(!n[t]){var o=n[t]={exports:{}};e[t][0].call(o.exports,function(n){var o=e[t][1][n];return r(o||n)},o,o.exports)}return n[t].exports}if("function"==typeof __nr_require)return __nr_require;for(var o=0;o<t.length;o++)r(t[o]);return r}({1:[function(e,n,t){function r(){}function o(e,n,t){return function(){return i(e,[c.now()].concat(u(arguments)),n?null:this,t),n?void 0:this}}var i=e("handle"),a=e(2),u=e(3),f=e("ee").get("tracer"),c=e("loader"),s=NREUM;"undefined"==typeof window.newrelic&&(newrelic=s);var p=["setPageViewName","setCustomAttribute","setErrorHandler","finished","addToTrace","inlineHit","addRelease"],d="api-",l=d+"ixn-";a(p,function(e,n){s[n]=o(d+n,!0,"api")}),s.addPageAction=o(d+"addPageAction",!0),s.setCurrentRouteName=o(d+"routeName",!0),n.exports=newrelic,s.interaction=function(){return(new r).get()};var m=r.prototype={createTracer:function(e,n){var t={},r=this,o="function"==typeof n;return i(l+"tracer",[c.now(),e,t],r),function(){if(f.emit((o?"":"no-")+"fn-start",[c.now(),r,o],t),o)try{return n.apply(this,arguments)}finally{f.emit("fn-end",[c.now()],t)}}}};a("setName,setAttribute,save,ignore,onEnd,getContext,end,get".split(","),function(e,n){m[n]=o(l+n)}),newrelic.noticeError=function(e){"string"==typeof e&&(e=new Error(e)),i("err",[e,c.now()])}},{}],2:[function(e,n,t){function r(e,n){var t=[],r="",i=0;for(r in e)o.call(e,r)&&(t[i]=n(r,e[r]),i+=1);return t}var o=Object.prototype.hasOwnProperty;n.exports=r},{}],3:[function(e,n,t){function r(e,n,t){n||(n=0),"undefined"==typeof t&&(t=e?e.length:0);for(var r=-1,o=t-n||0,i=Array(o<0?0:o);++r<o;)i[r]=e[n+r];return i}n.exports=r},{}],4:[function(e,n,t){n.exports={exists:"undefined"!=typeof window.performance&&window.performance.timing&&"undefined"!=typeof window.performance.timing.navigationStart}},{}],ee:[function(e,n,t){function r(){}function o(e){function n(e){return e&&e instanceof r?e:e?f(e,u,i):i()}function t(t,r,o,i){if(!d.aborted||i){e&&e(t,r,o);for(var a=n(o),u=m(t),f=u.length,c=0;c<f;c++)u[c].apply(a,r);var p=s[y[t]];return p&&p.push([b,t,r,a]),a}}function l(e,n){v[e]=m(e).concat(n)}function m(e){return v[e]||[]}function w(e){return p[e]=p[e]||o(t)}function g(e,n){c(e,function(e,t){n=n||"feature",y[t]=n,n in s||(s[n]=[])})}var v={},y={},b={on:l,emit:t,get:w,listeners:m,context:n,buffer:g,abort:a,aborted:!1};return b}function i(){return new r}function a(){(s.api||s.feature)&&(d.aborted=!0,s=d.backlog={})}var u="nr@context",f=e("gos"),c=e(2),s={},p={},d=n.exports=o();d.backlog=s},{}],gos:[function(e,n,t){function r(e,n,t){if(o.call(e,n))return e[n];var r=t();if(Object.defineProperty&&Object.keys)try{return Object.defineProperty(e,n,{value:r,writable:!0,enumerable:!1}),r}catch(i){}return e[n]=r,r}var o=Object.prototype.hasOwnProperty;n.exports=r},{}],handle:[function(e,n,t){function r(e,n,t,r){o.buffer([e],r),o.emit(e,n,t)}var o=e("ee").get("handle");n.exports=r,r.ee=o},{}],id:[function(e,n,t){function r(e){var n=typeof e;return!e||"object"!==n&&"function"!==n?-1:e===window?0:a(e,i,function(){return o++})}var o=1,i="nr@id",a=e("gos");n.exports=r},{}],loader:[function(e,n,t){function r(){if(!x++){var e=h.info=NREUM.info,n=d.getElementsByTagName("script")[0];if(setTimeout(s.abort,3e4),!(e&&e.licenseKey&&e.applicationID&&n))return s.abort();c(y,function(n,t){e[n]||(e[n]=t)}),f("mark",["onload",a()+h.offset],null,"api");var t=d.createElement("script");t.src="https://"+e.agent,n.parentNode.insertBefore(t,n)}}function o(){"complete"===d.readyState&&i()}function i(){f("mark",["domContent",a()+h.offset],null,"api")}function a(){return E.exists&&performance.now?Math.round(performance.now()):(u=Math.max((new Date).getTime(),u))-h.offset}var u=(new Date).getTime(),f=e("handle"),c=e(2),s=e("ee"),p=window,d=p.document,l="addEventListener",m="attachEvent",w=p.XMLHttpRequest,g=w&&w.prototype;NREUM.o={ST:setTimeout,SI:p.setImmediate,CT:clearTimeout,XHR:w,REQ:p.Request,EV:p.Event,PR:p.Promise,MO:p.MutationObserver};var v=""+location,y={beacon:"bam.nr-data.net",errorBeacon:"bam.nr-data.net",agent:"js-agent.newrelic.com/nr-1044.min.js"},b=w&&g&&g[l]&&!/CriOS/.test(navigator.userAgent),h=n.exports={offset:u,now:a,origin:v,features:{},xhrWrappable:b};e(1),d[l]?(d[l]("DOMContentLoaded",i,!1),p[l]("load",r,!1)):(d[m]("onreadystatechange",o),p[m]("onload",r)),f("mark",["firstbyte",u],null,"api");var x=0,E=e(4)},{}]},{},["loader"]);</script>
  



<meta id="bb-canon-url" name="bb-canon-url" content="https://bitbucket.org">
<meta name="bb-api-canon-url" content="https://api.bitbucket.org">


<meta name="bb-commit-hash" content="3d45b9426697">
<meta name="bb-app-node" content="app-164">
<meta name="bb-view-name" content="bitbucket.apps.wiki.views.wiki">
<meta name="ignore-whitespace" content="False">
<meta name="tab-size" content="None">
<meta name="locale" content="en">

<meta name="application-name" content="Bitbucket">
<meta name="apple-mobile-web-app-title" content="Bitbucket">


  
  <meta name="theme-color" content="#0049B0">
  <meta name="msapplication-TileColor" content="#0052CC">
  <meta name="msapplication-TileImage" content="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/img/logos/bitbucket/mstile-150x150.png">
  <link rel="apple-touch-icon" sizes="180x180" type="image/png" href="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/img/logos/bitbucket/apple-touch-icon.png">
  <link rel="icon" sizes="192x192" type="image/png" href="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/img/logos/bitbucket/android-chrome-192x192.png">
  <link rel="icon" sizes="16x16 24x24 32x32 64x64" type="image/x-icon" href="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/img/logos/bitbucket/favicon_2017.ico">
  <link rel="mask-icon" href="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/img/logos/bitbucket/safari-pinned-tab.svg" color="#0052CC">



<link rel="search" type="application/opensearchdescription+xml" href="/opensearch.xml" title="Bitbucket">

  <meta name="description" content="">
  
    
  



  <link rel="stylesheet" href="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/css/entry/vendor.css" />
<link rel="stylesheet" href="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/css/entry/app.css" />




  
  <script nonce="m8drC3HtUP8Uw7bR">
  window.__sentry__ = {"dsn": "https://ea49358f525d4019945839a3d7a8292a@sentry.io/159509", "release": "3d45b9426697 (production)", "tags": {"project": "bitbucket-core"}, "environment": "production"};
</script>
<script src="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/dist/webpack/sentry.js"></script>
  <script src="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/dist/webpack/early.js"></script>
  
  
    <link href="/phjounin/tftpd64/rss" rel="alternate nofollow" type="application/rss+xml" title="RSS feed for tftpd64" />
  

</head>
<body class="production aui-page-sidebar aui-sidebar-expanded"
    data-static-url="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/"
data-base-url="https://bitbucket.org"
data-no-avatar-image="img/default_avatar/user_blue.svg"
data-current-user="{&quot;isKbdShortcutsEnabled&quot;: true, &quot;isSshEnabled&quot;: false, &quot;isAuthenticated&quot;: false}"
data-atlassian-id="{&quot;loginStatusUrl&quot;: &quot;https://id.atlassian.com/profile/rest/profile&quot;}"
data-settings="{&quot;MENTIONS_MIN_QUERY_LENGTH&quot;: 3}"

data-current-repo="{&quot;scm&quot;: &quot;git&quot;, &quot;readOnly&quot;: false, &quot;mainbranch&quot;: {&quot;name&quot;: &quot;master&quot;}, &quot;language&quot;: &quot;c&quot;, &quot;owner&quot;: {&quot;username&quot;: &quot;phjounin&quot;, &quot;uuid&quot;: &quot;4916ed3b-3303-4fac-9b78-06a296fcb2a9&quot;, &quot;isTeam&quot;: false}, &quot;fullslug&quot;: &quot;phjounin/tftpd64&quot;, &quot;slug&quot;: &quot;tftpd64&quot;, &quot;id&quot;: 28088340, &quot;pygmentsLanguage&quot;: &quot;c&quot;}"






data-browser-monitoring="true"
data-switch-create-pullrequest-commit-status="true"

data-track-js-errors="true"


>
<div id="page">
  
    
    <div id="wrapper">
      
  
  


      
        <header id="header" role="banner" data-module="header/tracking">
          
  


          <nav class="aui-header aui-dropdown2-trigger-group" role="navigation">
            <div class="aui-header-inner">
              <div class="aui-header-primary">
                
  

                
                  <h1 class="aui-header-logo aui-header-logo-bitbucket logged-out" id="logo">
                    <a href="/">
                      <span class="aui-header-logo-device">Bitbucket</span>
                    </a>
                  </h1>
                
                
  

    
    
  

    
    
  
<ul class="aui-nav">
  
    <li>
      <a href="/product/features">
        Features
      </a>
    </li>
    <li>
      <a href="/plans">
        Pricing
      </a>
    </li>
  
</ul>

              </div>
              <div class="aui-header-secondary">
                
  

<ul role="menu" class="aui-nav">
  
  <li>
    <form action="/repo/all" method="get" class="aui-quicksearch">
      <label for="search-query" class="assistive">owner/repository</label>
      <input id="search-query" class="bb-repo-typeahead" type="text"
             placeholder="Find a repository&hellip;" name="name" autocomplete="off"
             data-bb-typeahead-focus="false">
    </form>
  </li>
  <li>
    <a id="help-menu-link" class="aui-nav-link" href="#"
        aria-controls="help-menu-dialog"
        data-aui-trigger>
      <span id="help-menu-icon" class="aui-icon aui-icon-small aui-iconfont-help"></span>
    </a>
  </li>
  
    <li>
      <a class="aui-dropdown2-trigger" href="#header-language"
          aria-controls="header-language" aria-owns="header-language"
          aria-haspopup="true" data-container="#header .aui-header-inner">
        <span>English</span></a>
      <nav id="header-language" class="aui-dropdown2 aui-style-default aui-dropdown2-radio aui-dropdown2-in-header"
          aria-hidden="true">
        <form method="post" action="/account/language/setlang/"
            data-module="i18n/header-language-form">
          <input type="hidden" name="language" value="">
          <ul>
          <li><a class="aui-dropdown2-radio interactive checked"
                data-value="en" href="#en">English</a></li>
          
          <li><a class="aui-dropdown2-radio interactive "
                data-value="ja" href="#ja">日本語</a></li>
          </ul>
        </form>
      </nav>
    </li>
  
  
      <li id="header-signup-button">
        <a id="sign-up-link" data-ct="bitbucket.header.signup.click" class="aui-button aui-button-primary" href="/account/signup/">
          Sign up
        </a>
      </li>
    <li id="user-options">
      <a href="/account/signin/?next=/phjounin/tftpd64/wiki/Download%2520Tftpd64.md" class="aui-nav-link login-link">Log in</a>
    </li>
  
</ul>

              </div>
            </div>
          </nav>
        </header>
      

      

      
  

<div id="account-warning" data-module="header/account-warning"
  data-unconfirmed-addresses="false"
  data-no-addresses="false"
  
></div>



      
  
<header id="aui-message-bar">
  
</header>


    <div id="content" role="main">
      
        
  
    <div class="aui-sidebar repo-sidebar"
    data-module="repo/sidebar"
  >
  <div class="aui-sidebar-wrapper">
    <div class="aui-sidebar-body">
      <header class="aui-page-header">
        <div class="aui-page-header-inner">
          <div class="aui-page-header-image">
            <a href="/phjounin/tftpd64" id="repo-avatar-link" class="repo-link">
              <span class="aui-avatar aui-avatar-large aui-avatar-project">
                <span class="aui-avatar-inner">
                  <img src="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/img/repo-avatars/c.svg" alt="">
                </span>
              </span>
            </a>
          </div>
          <div class="aui-page-header-main">
            <h1>
              
              <a href="/phjounin/tftpd64" title="tftpd64" class="entity-name">tftpd64</a>
            </h1>
          </div>
        </div>
      </header>
      <nav class="aui-navgroup aui-navgroup-vertical">
        <div class="aui-navgroup-inner">
          
            
              <div class="aui-sidebar-group aui-sidebar-group-actions repository-actions forks-enabled">
                <div class="aui-nav-heading">
                  <strong>Actions</strong>
                </div>
                <ul id="repo-actions" class="aui-nav">
                  
                  
                    <li>
                      <a id="repo-clone-button" class="aui-nav-item "
                        href="#clone"
                        data-ct="bitbucket.sidebar.actions.click"
                        data-ct-data="{&quot;label&quot;: &quot;repository.clone&quot;}"
                        data-module="components/clone/clone-dialog"
                        target="_self">
                        
                          <span class="aui-icon aui-icon-large icon-clone"></span>
                        
                        <span class="aui-nav-item-label">Clone</span>
                      </a>
                    </li>
                  
                    <li>
                      <a id="repo-compare-link" class="aui-nav-item "
                        href="/phjounin/tftpd64/branches/compare"
                        data-ct="bitbucket.sidebar.actions.click"
                        data-ct-data="{&quot;label&quot;: &quot;repository.compare&quot;}"
                        
                        target="_self">
                        
                          <span class="aui-icon aui-icon-large aui-icon-small aui-iconfont-devtools-compare"></span>
                        
                        <span class="aui-nav-item-label">Compare</span>
                      </a>
                    </li>
                  
                    <li>
                      <a id="repo-fork-link" class="aui-nav-item "
                        href="/phjounin/tftpd64/fork"
                        data-ct="bitbucket.sidebar.actions.click"
                        data-ct-data="{&quot;label&quot;: &quot;repository.fork&quot;}"
                        
                        target="_self">
                        
                          <span class="aui-icon aui-icon-large icon-fork"></span>
                        
                        <span class="aui-nav-item-label">Fork</span>
                      </a>
                    </li>
                  
                </ul>
              </div>
          
          <div class="aui-sidebar-group aui-sidebar-group-tier-one repository-sections">
            <div class="aui-nav-heading">
              <strong>Navigation</strong>
            </div>
            <ul class="aui-nav">
              
              
                <li>
                  <a id="repo-overview-link" class="aui-nav-item "
                    href="/phjounin/tftpd64/overview"
                    data-ct="bitbucket.sidebar.navigation.click"
                    data-ct-data="{&quot;label&quot;: &quot;repository.overview&quot;}"
                    
                    target="_self"
                    >
                    
                    
                      <span class="aui-icon aui-icon-large icon-overview"></span>
                    
                    <span class="aui-nav-item-label">
                      Overview
                      
                      
                    </span>
                  </a>
                </li>
              
                <li>
                  <a id="repo-source-link" class="aui-nav-item "
                    href="/phjounin/tftpd64/src"
                    data-ct="bitbucket.sidebar.navigation.click"
                    data-ct-data="{&quot;label&quot;: &quot;repository.source&quot;}"
                    
                    target="_self"
                    >
                    
                    
                      <span class="aui-icon aui-icon-large icon-source"></span>
                    
                    <span class="aui-nav-item-label">
                      Source
                      
                      
                    </span>
                  </a>
                </li>
              
                <li>
                  <a id="repo-commits-link" class="aui-nav-item "
                    href="/phjounin/tftpd64/commits/"
                    data-ct="bitbucket.sidebar.navigation.click"
                    data-ct-data="{&quot;label&quot;: &quot;repository.commits&quot;}"
                    
                    target="_self"
                    >
                    
                    
                      <span class="aui-icon aui-icon-large icon-commits"></span>
                    
                    <span class="aui-nav-item-label">
                      Commits
                      
                      
                    </span>
                  </a>
                </li>
              
                <li>
                  <a id="repo-branches-link" class="aui-nav-item "
                    href="/phjounin/tftpd64/branches/"
                    data-ct="bitbucket.sidebar.navigation.click"
                    data-ct-data="{&quot;label&quot;: &quot;repository.branches&quot;}"
                    
                    target="_self"
                    >
                    
                    
                      <span class="aui-icon aui-icon-large icon-branches"></span>
                    
                    <span class="aui-nav-item-label">
                      Branches
                      
                      
                    </span>
                  </a>
                </li>
              
                <li>
                  <a id="repo-pullrequests-link" class="aui-nav-item "
                    href="/phjounin/tftpd64/pull-requests/"
                    data-ct="bitbucket.sidebar.navigation.click"
                    data-ct-data="{&quot;label&quot;: &quot;repository.pullrequests&quot;}"
                    
                    target="_self"
                    >
                    
                    
                      <span class="aui-icon aui-icon-large icon-pull-requests"></span>
                    
                    <span class="aui-nav-item-label">
                      Pull requests
                      
                      
                    </span>
                  </a>
                </li>
              
                <li>
                  <a id="repopage-xxXyy7-add-on-link" class="aui-nav-item aui-nav-item add-on-menu-item"
                    href="/phjounin/tftpd64/addon/pipelines/home"
                    data-ct="bitbucket.sidebar.navigation.click"
                    data-ct-data="{&quot;addon_key&quot;: &quot;pipelines&quot;, &quot;module_key&quot;: &quot;home&quot;, &quot;label&quot;: &quot;user.addon&quot;}"
                    
                    target="_self"
                    >
                    
                    
                      <span class="aui-icon aui-icon-large aui-iconfont-build"></span>
                    
                    <span class="aui-nav-item-label">
                      Pipelines
                      
                      
                        
                          <span class="aui-lozenge aui-lozenge-current">New</span>
                        
                      
                    </span>
                  </a>
                </li>
              
                <li>
                  <a id="repo-issues-link" class="aui-nav-item "
                    href="/phjounin/tftpd64/issues?status=new&amp;status=open"
                    data-ct="bitbucket.sidebar.navigation.click"
                    data-ct-data="{&quot;label&quot;: &quot;issues&quot;}"
                    
                    target="_self"
                    title="( type &#39;r&#39; then &#39;i&#39; )">
                    
                    
                      <span class="aui-icon aui-icon-large icon-issues"></span>
                    
                    <span class="aui-nav-item-label">
                      Issues
                      
                      
                    </span>
                  </a>
                </li>
              
                <li class="aui-nav-selected">
                  <a id="repo-wiki-link" class="aui-nav-item "
                    href="/phjounin/tftpd64/wiki/"
                    data-ct="bitbucket.sidebar.navigation.click"
                    data-ct-data="{&quot;label&quot;: &quot;repository.wiki&quot;}"
                    
                    target="_self"
                    >
                    
                    
                      <span class="aui-icon aui-icon-large icon-wiki"></span>
                    
                    <span class="aui-nav-item-label">
                      Wiki
                      
                      
                    </span>
                  </a>
                </li>
              
                <li>
                  <a id="repo-downloads-link" class="aui-nav-item "
                    href="/phjounin/tftpd64/downloads/"
                    data-ct="bitbucket.sidebar.navigation.click"
                    data-ct-data="{&quot;label&quot;: &quot;repository.downloads&quot;}"
                    
                    target="_self"
                    >
                    
                    
                      <span class="aui-icon aui-icon-large icon-downloads"></span>
                    
                    <span class="aui-nav-item-label">
                      Downloads
                      
                      
                    </span>
                  </a>
                </li>
              
            </ul>
          </div>
          <div class="aui-sidebar-group aui-sidebar-group-tier-one repository-settings">
            <div class="aui-nav-heading">
              <strong class="assistive">Settings</strong>
            </div>
            <ul class="aui-nav">
              
              
            </ul>
          </div>
          
            
              <div class="hidden kb-shortcut-actions">
                <a id="repo-create-issue" href="/phjounin/tftpd64/issues/new"></a>
              </div>
            
          
        </div>
      </nav>
    </div>
    <div class="aui-sidebar-footer">
      <a class="aui-sidebar-toggle aui-sidebar-footer-tipsy aui-button aui-button-subtle"><span class="aui-icon"></span></a>
    </div>
  </div>
  

<div id="repo-clone-dialog" class="clone-dialog hidden">
  





  

<div class="clone-url" data-module="components/clone/url-dropdown" data-owner="4916ed3b-3303-4fac-9b78-06a296fcb2a9"
     data-location-context="header"
     data-fetch-url="https://bitbucket.org/phjounin/tftpd64.git"
     data-push-url="https://bitbucket.org/phjounin/tftpd64.git"
     
     
     
     >
  <div class="aui-buttons">
    
    <button class="aui-button aui-dropdown2-trigger protocol-trigger"
            data-command-prefix="git clone"
            data-primary-https="https://bitbucket.org/phjounin/tftpd64.git"
            data-primary-ssh="git@bitbucket.org:phjounin/tftpd64.git"
            aria-controls="protocols-list-header">
      <span class="dropdown-text">HTTPS</span>
    </button>
    <aui-dropdown-menu id="protocols-list-header" data-aui-alignment="bottom left">
      <aui-section id="protocols-list-section" class="aui-list-truncate">
        <aui-item-radio class="item-link https" checked>HTTPS</aui-item-radio>
        <aui-item-radio class="item-link ssh">SSH</aui-item-radio>
      </aui-section>
    </aui-dropdown-menu>
    <input type="text" readonly="readonly" class="clone-url-input"
           value="git clone https://bitbucket.org/phjounin/tftpd64.git">
  </div>
  
</div>

  <div class="learn-more">
    <p>Need help cloning? Learn how to
         <a href="https://confluence.atlassian.com/x/4whODQ" target="_blank">clone a repository</a>.
    </p>
  </div>
  
  <div class="sourcetree-callout clone-in-sourcetree"
  data-module="components/clone/clone-in-sourcetree"
  data-https-url="https://bitbucket.org/phjounin/tftpd64.git"
  data-ssh-url="ssh://git@bitbucket.org/phjounin/tftpd64.git">

  <div>
    <button class="aui-button aui-button-primary">
      
        Clone in SourceTree
      
    </button>
  </div>

  <p class="windows-text">
    
      <a href="http://www.sourcetreeapp.com/?utm_source=internal&amp;utm_medium=link&amp;utm_campaign=clone_repo_win" target="_blank">Atlassian SourceTree</a>
      is a free Git and Mercurial client for Windows.
    
  </p>
  <p class="mac-text">
    
      <a href="http://www.sourcetreeapp.com/?utm_source=internal&amp;utm_medium=link&amp;utm_campaign=clone_repo_mac" target="_blank">Atlassian SourceTree</a>
      is a free Git and Mercurial client for Mac.
    
  </p>
</div>
</div>
</div>
  

        
  <div class="aui-page-panel ">
    <div class="hidden">


</div>
    <div class="aui-page-panel-inner">
      <div
        id="repo-content"
        class="aui-page-panel-content forks-enabled"
        data-module="repo/index"
        
      >
        
          
            <ol class="aui-nav aui-nav-breadcrumbs">
              <li>
  <a href="/phjounin/">Philippe Jounin</a>
</li>

<li>
  <a href="/phjounin/tftpd64">tftpd64</a>
</li>
              
            </ol>
          
          <div class="aui-group repo-page-header">
            <div class="aui-item section-title">
              <h1>Wiki</h1>
            </div>
            <div class="aui-item page-actions">
              
  
  <a id="clone-wiki-contextual" data-module="components/clone/clone-wiki-contextual"
     href="https://bitbucket.org/phjounin/tftpd64.git/wiki" class="aui-button aui-button-subtle">
    <span class="aui-icon aui-icon-small aui-iconfont-devtools-clone"></span>
    Clone wiki
  </a>
  

<div id="wiki-clone-dialog" class="clone-dialog hidden">
  





  
  

<div class="clone-url" data-module="components/clone/url-dropdown" data-owner="4916ed3b-3303-4fac-9b78-06a296fcb2a9"
     data-location-context="wiki"
     data-fetch-url="https://bitbucket.org/phjounin/tftpd64.git/wiki"
     data-push-url="https://bitbucket.org/phjounin/tftpd64.git/wiki"
     data-clone-path="/wiki"
     
     
     >
  <div class="aui-buttons">
    
    <button class="aui-button aui-dropdown2-trigger protocol-trigger"
            data-command-prefix="git clone"
            data-primary-https="https://bitbucket.org/phjounin/tftpd64.git/wiki"
            data-primary-ssh="git@bitbucket.org:phjounin/tftpd64.git/wiki"
            aria-controls="protocols-list-wiki">
      <span class="dropdown-text">HTTPS</span>
    </button>
    <aui-dropdown-menu id="protocols-list-wiki" data-aui-alignment="bottom left">
      <aui-section id="protocols-list-section" class="aui-list-truncate">
        <aui-item-radio class="item-link https" checked>HTTPS</aui-item-radio>
        <aui-item-radio class="item-link ssh">SSH</aui-item-radio>
      </aui-section>
    </aui-dropdown-menu>
    <input type="text" readonly="readonly" class="clone-url-input"
           value="git clone https://bitbucket.org/phjounin/tftpd64.git/wiki">
  </div>
  
</div>

  <div class="learn-more">
    <p>Need help cloning? Learn how to
       <a href="https://confluence.atlassian.com/x/4whODQ" target="_blank">clone a repository</a>.
    </p>
  </div>
  <div class="sourcetree-callout clone-in-sourcetree"
  data-module="components/clone/clone-in-sourcetree"
  data-https-url="https://bitbucket.org/phjounin/tftpd64.git"
  data-ssh-url="ssh://git@bitbucket.org/phjounin/tftpd64.git">

  <div>
    <button class="aui-button aui-button-primary">
      
        Clone in SourceTree
      
    </button>
  </div>

  <p class="windows-text">
    
      <a href="http://www.sourcetreeapp.com/?utm_source=internal&amp;utm_medium=link&amp;utm_campaign=clone_repo_win" target="_blank">Atlassian SourceTree</a>
      is a free Git and Mercurial client for Windows.
    
  </p>
  <p class="mac-text">
    
      <a href="http://www.sourcetreeapp.com/?utm_source=internal&amp;utm_medium=link&amp;utm_campaign=clone_repo_mac" target="_blank">Atlassian SourceTree</a>
      is a free Git and Mercurial client for Mac.
    
  </p>
</div>
</div>

            </div>
          </div>
        
        
  <section id="wiki">
    <header>
      <h1>
        

<a href="/phjounin/tftpd64/wiki/browse/" class="root">tftpd64</a>
/

  
    
      <span class="file-name">Download Tftpd64</span>
    
  

      </h1>
      


  <div class="wiki-toolbar">
    <div class="aui-buttons">
      <a href="/phjounin/tftpd64/wiki/Download%20Tftpd64" class="aui-button" aria-pressed="true">View</a>
      <a href="/phjounin/tftpd64/wiki/history/Download%20Tftpd64" class="aui-button history">History</a>
      
    </div>
    
  </div>



    </header>
    
      
        <section id="wiki-content" class="wiki-content">
          
            <p>Recommended Download  </p>
<blockquote>
<p><a href="https://bitbucket.org/phjounin/tftpd64/downloads/Tftpd64-4.60-setup.exe">Application tftpd64 with installer</a>  </p>
</blockquote>
<p>Other Available Downloads  </p>
<blockquote>
<p><a href="https://bitbucket.org/phjounin/tftpd64/downloads/Tftpd64_SE-4.60-setup.exe">Application tftpd64 portable edition</a><br />
<a href="https://bitbucket.org/phjounin/tftpd64/downloads/tftpd64.v460.zip">Application tftpd64 service edition</a><br />
<a href="https://bitbucket.org/phjounin/tftpd64/downloads/Tftpd32-4.60-setup.exe">Application tftpd32 with installer</a><br />
<a href="https://bitbucket.org/phjounin/tftpd64/downloads/tftpd32.v460.zip">Application tftpd32 portable edition</a><br />
<a href="https://bitbucket.org/phjounin/tftpd64/downloads/Tftpd32_SE-4.60-setup.exe">Application tftpd32 service edition</a></p>
</blockquote>
          
        </section>
      
    

    
      <p class="revision-date">
        
          Updated <time datetime="2017-09-11T07:18:58+00:00">an hour ago</time>
        
      </p>
    
  </section>

        
        
        
      </div>
    </div>
    <div id="code-search-cta"></div>
  </div>

      </div>
    </div>
  

  
    <footer id="footer"  class="has-2017-logos"  role="contentinfo" data-module="components/footer">
      <section class="footer-body">
        
  <ul>
  <li>
    <a class="support-ga" target="_blank"
       data-support-gaq-page="Blog"
       href="http://blog.bitbucket.org">Blog</a>
  </li>
  <li>
    <a class="support-ga" target="_blank"
       data-support-gaq-page="Home"
       href="https://support.atlassian.com/bitbucket-cloud/">Support</a>
  </li>
  <li>
    <a class="support-ga"
       data-support-gaq-page="PlansPricing"
       href="/plans">Plans &amp; pricing</a>
  </li>
  <li>
    <a class="support-ga" target="_blank"
       data-support-gaq-page="DocumentationHome"
       href="//confluence.atlassian.com/display/BITBUCKET">Documentation</a>
  </li>
  <li>
    <a class="support-ga" target="_blank"
       data-support-gaq-page="DocumentationAPI"
       href="https://developer.atlassian.com/bitbucket/api/2/reference/">API</a>
  </li>
  <li>
    <a class="support-ga" target="_blank"
       data-support-gaq-page="SiteStatus"
       href="https://status.bitbucket.org/">Site status</a>
  </li>
  <li>
    <a class="support-ga" id="meta-info"
       data-support-gaq-page="MetaInfo"
       href="#">Version info</a>
  </li>
  <li>
    <a class="support-ga" target="_blank"
       data-support-gaq-page="EndUserAgreement"
       href="//www.atlassian.com/legal/customer-agreement?utm_source=bitbucket&amp;utm_medium=link&amp;utm_campaign=footer">Terms of service</a>
  </li>
  <li>
    <a class="support-ga" target="_blank"
       data-support-gaq-page="PrivacyPolicy"
       href="//www.atlassian.com/legal/privacy-policy">Privacy policy</a>
  </li>
</ul>
<div id="meta-info-content" style="display: none;">
  <ul>
    
      <li>English</li>
    
    <li>
      <a class="support-ga" target="_blank"
         data-support-gaq-page="GitDocumentation"
         href="http://git-scm.com/">Git 2.7.4.1.g5468f9e</a>
    </li>
    <li>
      <a class="support-ga" target="_blank"
         data-support-gaq-page="HgDocumentation"
         href="https://www.mercurial-scm.org">Mercurial 4.2</a>
    </li>
    <li>
      <a class="support-ga" target="_blank"
         data-support-gaq-page="DjangoDocumentation"
         href="https://www.djangoproject.com/">Django 1.9.12</a>
    </li>
    <li>
      <a class="support-ga" target="_blank"
         data-support-gaq-page="PythonDocumentation"
         href="http://www.python.org/">Python 2.7.13</a>
    </li>
    <li>
      <a class="support-ga" target="_blank"
         data-support-gaq-page="DeployedVersion"
         data-media-hex="3d45b9426697"
         href="#">3d45b9426697 / 3d45b9426697 @ app-164</a>
    </li>
    <li>
      <a class="support-ga" target="_blank"
         data-support-gaq-page="StorageRegion"
         href="#">Region production</a>
    </li>
  </ul>
</div>
<ul class="atlassian-links">
  <li>
    <a id="atlassian-jira-link" target="_blank"
       title="Track everything – bugs, tasks, deadlines, code – and pull reports to stay informed."
       href="https://www.atlassian.com/software/jira/bitbucket-integration?utm_source=bitbucket&amp;utm_medium=link&amp;utm_campaign=bitbucket_footer">JIRA Software</a>
  </li>
  <li>
    <a id="atlassian-confluence-link" target="_blank"
       title="Content Creation, Collaboration & Knowledge Sharing for Teams."
       href="http://www.atlassian.com/software/confluence/overview/team-collaboration-software?utm_source=bitbucket&amp;utm_medium=link&amp;utm_campaign=bitbucket_footer">Confluence</a>
  </li>
  <li>
    <a id="atlassian-bamboo-link" target="_blank"
       title="Continuous integration and deployment, release management."
       href="http://www.atlassian.com/software/bamboo?utm_source=bitbucket&amp;utm_medium=link&amp;utm_campaign=bitbucket_footer">Bamboo</a>
  </li>
  <li>
    <a id="atlassian-sourcetree-link" target="_blank"
       title="A free Git and Mercurial desktop client for Mac or Windows."
       href="http://www.sourcetreeapp.com/?utm_source=bitbucket&amp;utm_medium=link&amp;utm_campaign=bitbucket_footer">SourceTree</a>
  </li>
  <li>
    <a id="atlassian-hipchat-link" target="_blank"
       title="Group chat and IM built for teams."
       href="http://www.hipchat.com/?utm_source=bitbucket&amp;utm_medium=link&amp;utm_campaign=bitbucket_footer">HipChat</a>
  </li>
</ul>
<div id="footer-logo">
  <a target="_blank"
     title="Bitbucket is developed by Atlassian in Austin, San Francisco, and Sydney."
     href="http://www.atlassian.com?utm_source=bitbucket&amp;utm_medium=logo&amp;utm_campaign=bitbucket_footer">Atlassian</a>
</div>

      </section>
    </footer>
  
</div>



  

<div data-module="components/mentions/index">
  
    
    
  
  
    
    
  
  
    
    
  
</div>
<div data-module="components/typeahead/emoji/index">
  
    
    
  
</div>

<div data-module="components/repo-typeahead/index">
  
    
    
  
</div>

    
    
  

    
    
  

    
    
  

    
    
  


  <aui-inline-dialog
    id="help-menu-dialog"
    data-aui-alignment="bottom right"

    
    data-aui-alignment-static="true"
    data-module="header/help-menu"
    responds-to="toggle"
    aria-hidden="true">

  <div id="help-menu-section">
    <h1 class="help-menu-heading">Help</h1>

    <form id="help-menu-search-form" class="aui" target="_blank" method="get"
        action="https://support.atlassian.com/customer/search">
      <span id="help-menu-search-icon" class="aui-icon aui-icon-large aui-iconfont-search"></span>
      <input id="help-menu-search-form-input" name="q" class="text" type="text" placeholder="Ask a question">
    </form>

    <ul id="help-menu-links">
      <li>
        <a class="support-ga" data-support-gaq-page="DocumentationHome"
            href="https://confluence.atlassian.com/x/bgozDQ" target="_blank">
          Online help
        </a>
      </li>
      <li>
        <a class="support-ga" data-support-gaq-page="GitTutorials"
            href="https://www.atlassian.com/git?utm_source=bitbucket&amp;utm_medium=link&amp;utm_campaign=help_dropdown&amp;utm_content=learn_git"
            target="_blank">
          Learn Git
        </a>
      </li>
      <li>
        <a id="keyboard-shortcuts-link"
           href="#">Keyboard shortcuts</a>
      </li>
      <li>
        <a class="support-ga" data-support-gaq-page="DocumentationTutorials"
            href="https://confluence.atlassian.com/x/Q4sFLQ" target="_blank">
          Bitbucket tutorials
        </a>
      </li>
      <li>
        <a class="support-ga" data-support-gaq-page="SiteStatus"
            href="https://status.bitbucket.org/" target="_blank">
          Site status
        </a>
      </li>
      <li>
        <a class="support-ga" data-support-gaq-page="Home"
            href="https://support.atlassian.com/bitbucket-cloud/" target="_blank">
          Support
        </a>
      </li>
    </ul>
  </div>
</aui-inline-dialog>
  





  

  <div class="_mustache-templates">
    
      <script id="repo-dropdown-template" type="text/html">
        


[[#hasViewed]]
  <div class="aui-dropdown2-section">
    <strong class="viewed">Recently viewed</strong>
    <ul>
      [[#viewed]]
        <li class="[[#is_private]]private[[/is_private]][[^is_private]]public[[/is_private]] repository">
          <a href="[[url]]" title="[[owner]][[#project]] / [[project]][[/project]] / [[name]]" class="aui-icon-container recently-viewed repo-link"
              data-ct="bitbucket.header.repository.dropdown.repository.click" data-ct-data='{"type": "recently-viewed"}'>
            <span class="aui-avatar aui-avatar-xsmall aui-avatar-project">
              <span class="aui-avatar-inner">
                <img src="[[{avatar}]]">
              </span>
            </span>
            <span class="dropdown-path">
              <span class="dropdown-path-part">[[#project]][[project]][[/project]][[^project]][[owner]][[/project]]</span>
              <span class="dropdown-path-separator">/</span>
              <span class="dropdown-path-part dropdown-path-part--primary">[[name]]</span>
            </span>
          </a>
        </li>
      [[/viewed]]
    </ul>
  </div>
[[/hasViewed]]
[[#hasUpdated]]
  <div class="aui-dropdown2-section">
    <strong class="updated">Recently updated</strong>
    <ul>
      [[#updated]]
      <li class="[[#is_private]]private[[/is_private]][[^is_private]]public[[/is_private]] repository">
        <a href="[[url]]" title="[[owner]][[#project]] / [[project]][[/project]] / [[name]]" class="aui-icon-container recently-updated repo-link"
           data-ct="bitbucket.header.repository.dropdown.repository.click" data-ct-data='{"type": "recently-updated"}'>
          <span class="aui-avatar aui-avatar-xsmall aui-avatar-project">
            <span class="aui-avatar-inner">
              <img src="[[{avatar}]]">
            </span>
          </span>
          <span class="dropdown-path">
            <span class="dropdown-path-part">[[#project]][[project]][[/project]][[^project]][[owner]][[/project]]</span>
            <span class="dropdown-path-separator">/</span>
            <span class="dropdown-path-part dropdown-path-part--primary">[[name]]</span>
          </span>
        </a>
      </li>
      [[/updated]]
    </ul>
  </div>
[[/hasUpdated]]

      </script>
    
      <script id="snippet-dropdown-template" type="text/html">
        <div class="aui-dropdown2-section">
  <strong>[[sectionTitle]]</strong>
  <ul class="aui-list-truncate">
    [[#snippets]]
      <li>
        <a href="[[links.html.href]]">[[owner.display_name]] / [[name]]</a>
      </li>
    [[/snippets]]
  </ul>
</div>

      </script>
    
      <script id="mention-result" type="text/html">
        
<span class="mention-result">
  <span class="aui-avatar aui-avatar-small mention-result--avatar">
    <span class="aui-avatar-inner">
      <img src="[[avatar_url]]">
    </span>
  </span>
  [[#display_name]]
    <span class="display-name mention-result--display-name">[[&display_name]]</span> <small class="username mention-result--username">[[&username]]</small>
  [[/display_name]]
  [[^display_name]]
    <span class="username mention-result--username">[[&username]]</span>
  [[/display_name]]
  [[#is_teammate]][[^is_team]]
    <span class="aui-lozenge aui-lozenge-complete aui-lozenge-subtle mention-result--lozenge">teammate</span>
  [[/is_team]][[/is_teammate]]
</span>
      </script>
    
      <script id="mention-call-to-action" type="text/html">
        
[[^query]]
<li class="bb-typeahead-item">Begin typing to search for a user</li>
[[/query]]
[[#query]]
<li class="bb-typeahead-item">Continue typing to search for a user</li>
[[/query]]

      </script>
    
      <script id="mention-no-results" type="text/html">
        
[[^searching]]
<li class="bb-typeahead-item">Found no matching users for <em>[[query]]</em>.</li>
[[/searching]]
[[#searching]]
<li class="bb-typeahead-item bb-typeahead-searching">Searching for <em>[[query]]</em>.</li>
[[/searching]]

      </script>
    
      <script id="emoji-result" type="text/html">
        
<span class="emoji-result">
  <span class="emoji-result--avatar">
    <img class="emoji" src="[[src]]">
  </span>
  <span class="name emoji-result--name">[[&name]]</span>
</span>

      </script>
    
      <script id="repo-typeahead-result" type="text/html">
        <span class="aui-avatar aui-avatar-project aui-avatar-xsmall">
  <span class="aui-avatar-inner">
    <img src="[[avatar]]">
  </span>
</span>
<span class="owner">[[&owner]]</span>/<span class="slug">[[&slug]]</span>

      </script>
    
      <script id="share-form-template" type="text/html">
        

<div class="error aui-message hidden">
  <span class="aui-icon icon-error"></span>
  <div class="message"></div>
</div>
<form class="aui">
  <table class="widget bb-list aui">
    <thead>
    <tr class="assistive">
      <th class="user">User</th>
      <th class="role">Role</th>
      <th class="actions">Actions</th>
    </tr>
    </thead>
    <tbody>
      <tr class="form">
        <td colspan="2">
          <input type="text" class="text bb-user-typeahead user-or-email"
                 placeholder="Username or email address"
                 autocomplete="off"
                 data-bb-typeahead-focus="false"
                 [[#disabled]]disabled[[/disabled]]>
        </td>
        <td class="actions">
          <button type="submit" class="aui-button aui-button-light" disabled>Add</button>
        </td>
      </tr>
    </tbody>
  </table>
</form>

      </script>
    
      <script id="share-detail-template" type="text/html">
        

[[#username]]
<td class="user
    [[#hasCustomGroups]]custom-groups[[/hasCustomGroups]]"
    [[#error]]data-error="[[error]]"[[/error]]>
  <div title="[[displayName]]">
    <a href="/[[username]]/" class="user">
      <span class="aui-avatar aui-avatar-xsmall">
        <span class="aui-avatar-inner">
          <img src="[[avatar]]">
        </span>
      </span>
      <span>[[displayName]]</span>
    </a>
  </div>
</td>
[[/username]]
[[^username]]
<td class="email
    [[#hasCustomGroups]]custom-groups[[/hasCustomGroups]]"
    [[#error]]data-error="[[error]]"[[/error]]>
  <div title="[[email]]">
    <span class="aui-icon aui-icon-small aui-iconfont-email"></span>
    [[email]]
  </div>
</td>
[[/username]]
<td class="role
    [[#hasCustomGroups]]custom-groups[[/hasCustomGroups]]">
  <div>
    [[#group]]
      [[#hasCustomGroups]]
        <select class="group [[#noGroupChoices]]hidden[[/noGroupChoices]]">
          [[#groups]]
            <option value="[[slug]]"
                [[#isSelected]]selected[[/isSelected]]>
              [[name]]
            </option>
          [[/groups]]
        </select>
      [[/hasCustomGroups]]
      [[^hasCustomGroups]]
      <label>
        <input type="checkbox" class="admin"
            [[#isAdmin]]checked[[/isAdmin]]>
        Administrator
      </label>
      [[/hasCustomGroups]]
    [[/group]]
    [[^group]]
      <ul>
        <li class="permission aui-lozenge aui-lozenge-complete
            [[^read]]aui-lozenge-subtle[[/read]]"
            data-permission="read">
          read
        </li>
        <li class="permission aui-lozenge aui-lozenge-complete
            [[^write]]aui-lozenge-subtle[[/write]]"
            data-permission="write">
          write
        </li>
        <li class="permission aui-lozenge aui-lozenge-complete
            [[^admin]]aui-lozenge-subtle[[/admin]]"
            data-permission="admin">
          admin
        </li>
      </ul>
    [[/group]]
  </div>
</td>
<td class="actions
    [[#hasCustomGroups]]custom-groups[[/hasCustomGroups]]">
  <div>
    <a href="#" class="delete">
      <span class="aui-icon aui-icon-small aui-iconfont-remove">Delete</span>
    </a>
  </div>
</td>

      </script>
    
      <script id="share-team-template" type="text/html">
        

<div class="clearfix">
  <span class="team-avatar-container">
    <span class="aui-avatar aui-avatar-medium">
      <span class="aui-avatar-inner">
        <img src="[[avatar]]">
      </span>
    </span>
  </span>
  <span class="team-name-container">
    [[display_name]]
  </span>
</div>
<p class="helptext">
  
    Existing users are granted access to this team immediately.
    New users will be sent an invitation.
  
</p>
<div class="share-form"></div>

      </script>
    
      <script id="scope-list-template" type="text/html">
        <ul class="scope-list">
  [[#scopes]]
    <li class="scope-list--item">
      <span class="scope-list--icon aui-icon aui-icon-small [[icon]]"></span>
      <span class="scope-list--description">[[description]]</span>
    </li>
  [[/scopes]]
</ul>

      </script>
    
  </div>




  
    <script nonce="m8drC3HtUP8Uw7bR">
  window.__initial_state__ = {"global": {"features": {"pr-merge-sign-off": true, "comment-likes": true, "fast-pr-merges": true, "twofactor-u2f": true, "clone-mirrors": true, "repo-onboarding": true, "require-issue-key": true, "snippets": true, "app-passwords": true, "fabric-emoji": true, "revert-pull-request": true, "new-dashboard": true, "integrations-page": true, "online-ides": true, "new-signup-flow": true, "fe_word_diff": true, "use-moneybucket": true, "pinned-issue-comments": true, "invitation-matching-email": true, "2017-logos": true, "diff-renames-public": true, "ignore-whitespace-button": true, "trello-boards": true, "squash-merge": true, "pr-shared-component": true, "build-status": true, "code-search-cta": true, "projects": true, "pr-checkout-command": true, "new-features-management": true, "repo-avatar-revival": true, "diff-renames-internal": true, "search-syntax-highlighting": true, "code-search-cta-launch": true, "new-features": true, "promote-bello": true, "search-results-adg3-page": true, "lfs_post_beta": true, "rollout-bello": true, "two-step": true, "bundle-splitting": true}, "locale": "en", "geoip_country": null, "targetFeatures": {"pr-merge-sign-off": true, "comment-likes": true, "fast-pr-merges": true, "twofactor-u2f": true, "clone-mirrors": true, "repo-onboarding": true, "require-issue-key": true, "snippets": true, "adg3": true, "fabric-emoji": true, "revert-pull-request": true, "new-dashboard": true, "integrations-page": true, "online-ides": true, "new-signup-flow": true, "fe_word_diff": true, "clonebundles": true, "use-moneybucket": true, "pinned-issue-comments": true, "invitation-matching-email": true, "2017-logos": true, "diff-renames-public": true, "app-passwords": true, "ignore-whitespace-button": true, "trello-boards": true, "atlassian-editor": false, "squash-merge": true, "pr-shared-component": true, "build-status": true, "code-search-cta": true, "projects": true, "evolution": false, "pr-checkout-command": true, "new-features-management": true, "repo-avatar-revival": true, "diff-renames-internal": true, "search-syntax-highlighting": true, "code-search-cta-launch": true, "new-features": true, "promote-bello": true, "search-results-adg3-page": true, "lfs_post_beta": true, "rollout-bello": true, "two-step": true, "bundle-splitting": true}, "isFocusedTask": false, "targetUser": {"username": "phjounin", "website": null, "display_name": "Philippe Jounin", "account_id": "557058:51ddb9f4-5044-4ff0-9c97-eeaad25dd594", "links": {"self": {"href": "https://bitbucket.org/!api/2.0/users/phjounin"}, "html": {"href": "https://bitbucket.org/phjounin/"}, "avatar": {"href": "https://bitbucket.org/account/phjounin/avatar/32/"}}, "extra": {"has_atlassian_account": true}, "created_on": "2017-09-07T13:14:55.299703+00:00", "is_staff": false, "location": null, "type": "user", "uuid": "{4916ed3b-3303-4fac-9b78-06a296fcb2a9}"}, "path": "/phjounin/tftpd64/wiki/Download Tftpd64.md", "focusedTaskBackButtonUrl": null}, "repository": {"section": {"connectActions": [], "cloneProtocol": "https", "currentRepository": {"scm": "git", "name": "tftpd64", "links": {"clone": [{"href": "https://bitbucket.org/phjounin/tftpd64.git", "name": "https"}, {"href": "ssh://git@bitbucket.org/phjounin/tftpd64.git", "name": "ssh"}], "self": {"href": "https://bitbucket.org/!api/2.0/repositories/phjounin/tftpd64"}, "html": {"href": "https://bitbucket.org/phjounin/tftpd64"}, "avatar": {"href": "https://bitbucket.org/phjounin/tftpd64/avatar/32/"}}, "full_name": "phjounin/tftpd64", "owner": {"username": "phjounin", "website": null, "display_name": "Philippe Jounin", "account_id": "557058:51ddb9f4-5044-4ff0-9c97-eeaad25dd594", "links": {"self": {"href": "https://bitbucket.org/!api/2.0/users/phjounin"}, "html": {"href": "https://bitbucket.org/phjounin/"}, "avatar": {"href": "https://bitbucket.org/account/phjounin/avatar/32/"}}, "created_on": "2017-09-07T13:14:55.299703+00:00", "is_staff": false, "location": null, "type": "user", "uuid": "{4916ed3b-3303-4fac-9b78-06a296fcb2a9}"}, "type": "repository", "slug": "tftpd64", "uuid": "{0d2c3bf4-a97c-40b5-b347-2c7c47f5335d}"}, "menuItems": [{"analytics_label": "repository.overview", "icon_class": "icon-overview", "badge_label": null, "weight": 100, "url": "/phjounin/tftpd64/overview", "tab_name": "overview", "can_display": true, "label": "Overview", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-overview-link", "icon": "icon-overview"}, {"analytics_label": "repository.source", "icon_class": "icon-source", "badge_label": null, "weight": 200, "url": "/phjounin/tftpd64/src", "tab_name": "source", "can_display": true, "label": "Source", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-source-link", "icon": "icon-source"}, {"analytics_label": "repository.commits", "icon_class": "icon-commits", "badge_label": null, "weight": 300, "url": "/phjounin/tftpd64/commits/", "tab_name": "commits", "can_display": true, "label": "Commits", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-commits-link", "icon": "icon-commits"}, {"analytics_label": "repository.branches", "icon_class": "icon-branches", "badge_label": null, "weight": 400, "url": "/phjounin/tftpd64/branches/", "tab_name": "branches", "can_display": true, "label": "Branches", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-branches-link", "icon": "icon-branches"}, {"analytics_label": "repository.pullrequests", "icon_class": "icon-pull-requests", "badge_label": "0 open pull requests", "weight": 500, "url": "/phjounin/tftpd64/pull-requests/", "tab_name": "pullrequests", "can_display": true, "label": "Pull requests", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-pullrequests-link", "icon": "icon-pull-requests"}, {"analytics_label": "user.addon", "icon_class": "aui-iconfont-build", "badge_label": null, "weight": 550, "url": "/phjounin/tftpd64/addon/pipelines/home", "tab_name": "repopage-xxXyy7-add-on-link", "can_display": true, "label": "Pipelines", "anchor": true, "analytics_payload": {}, "icon_url": null, "type": "connect_menu_item", "id": "repopage-xxXyy7-add-on-link", "target": "_self"}, {"analytics_label": "issues", "icon_class": "icon-issues", "badge_label": "0 active issues", "weight": 600, "url": "/phjounin/tftpd64/issues?status=new&status=open", "tab_name": "issues", "can_display": true, "label": "Issues", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-issues-link", "icon": "icon-issues"}, {"analytics_label": "repository.wiki", "icon_class": "icon-wiki", "badge_label": null, "weight": 700, "url": "/phjounin/tftpd64/wiki/", "tab_name": "wiki", "can_display": true, "label": "Wiki", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-wiki-link", "icon": "icon-wiki"}, {"analytics_label": "repository.downloads", "icon_class": "icon-downloads", "badge_label": null, "weight": 800, "url": "/phjounin/tftpd64/downloads/", "tab_name": "downloads", "can_display": true, "label": "Downloads", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-downloads-link", "icon": "icon-downloads"}], "bitbucketActions": [{"analytics_label": "repository.clone", "icon_class": "icon-clone", "badge_label": null, "weight": 100, "url": "#clone", "tab_name": "clone", "can_display": true, "label": "Clone", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-clone-button", "icon": "icon-clone"}, {"analytics_label": "repository.compare", "icon_class": "aui-icon-small aui-iconfont-devtools-compare", "badge_label": null, "weight": 400, "url": "/phjounin/tftpd64/branches/compare", "tab_name": "compare", "can_display": true, "label": "Compare", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-compare-link", "icon": "aui-icon-small aui-iconfont-devtools-compare"}, {"analytics_label": "repository.fork", "icon_class": "icon-fork", "badge_label": null, "weight": 500, "url": "/phjounin/tftpd64/fork", "tab_name": "fork", "can_display": true, "label": "Fork", "anchor": true, "analytics_payload": {}, "target": "_self", "type": "menu_item", "id": "repo-fork-link", "icon": "icon-fork"}], "activeMenuItem": "wiki"}}};
  window.__settings__ = {"SOCIAL_AUTH_ATLASSIANID_LOGOUT_URL": "https://id.atlassian.com/logout", "CANON_URL": "https://bitbucket.org", "API_CANON_URL": "https://api.bitbucket.org"};
</script>
<script src="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/jsi18n/en/djangojs.js"></script>

  <script src="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/dist/webpack/locales/en.js"></script>

<script src="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/dist/webpack/common.js"></script>
<script src="https://d301sr5gafysq2.cloudfront.net/3d45b9426697/dist/webpack/wiki.js"></script>
  


<script async src="https://www.google-analytics.com/analytics.js"></script>

<script type="text/javascript">window.NREUM||(NREUM={});NREUM.info={"beacon":"bam.nr-data.net","queueTime":0,"licenseKey":"a2cef8c3d3","agent":"","transactionName":"Z11RZxdWW0cEVkYLDV4XdUYLVEFdClsdAAtEWkZQDlJBGgRFQhFMR1FYWktBXFESRggVC1tR","applicationID":"1841284","errorBeacon":"bam.nr-data.net","applicationTime":318}</script>
</body>
</html>
