<header id="loris-header">
</header>
<script type="text/javascript">
document.addEventListener('DOMContentLoaded', () => {
    ReactDOM.createRoot(
        document.getElementById('loris-header')
    ).render(
        React.createElement(Header, {
            sandbox: {if $sandbox|default} true {else} false {/if},
            categories: {$menus|@json_encode},
            controlPanel: {if $control_panel|default} true {else} false {/if},
            feedbackPanel: {if $bvl_feedback|default} true {else} false {/if},
            userName: "{$user.Real_name|escape}",
            userPrefs: {$my_preferences},
            sitesCount: {$userNumSites},
            sitesTooltop: "{$user.SitesTooltip|escape}",
        })
    );
});
</script>
