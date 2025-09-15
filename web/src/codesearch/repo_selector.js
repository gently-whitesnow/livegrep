var $ = require('jquery');
var _ = require('underscore');

function init() {
    $('#repos').selectpicker({
        actionsBox: true,
        selectedTextFormat: 'count > 4',
        countSelectedText: '({0} repositories)',
        noneSelectedText: '(all repositories)',
        liveSearch: true,
        width: '20em'
    });
    $('#repos').on('refreshed.bs.select', function () {
        var headers = $(this).parent().find('.dropdown-header');
        headers.css('cursor', 'pointer');
        headers.on('click', function (event) {
            event.stopPropagation();
            var optgroup = $('#repos optgroup[label="' + $(this).text() + '"]')
            var allSelected = !optgroup.children('option:not(:selected)').length;
            optgroup.children().prop('selected', !allSelected);
            $("#repos").selectpicker('refresh').trigger("change");
        });
    });
    $(window).on('keyup', '.bootstrap-select .bs-searchbox input', function(event) {
        var keycode = (event.keyCode ? event.keyCode : event.which);
        if(keycode == '13'){
            $(this).val("");
            $("#repos").selectpicker('refresh');
        }
    });
    $(window).keyup(function (keyevent) {
        var code = (keyevent.keyCode ? keyevent.keyCode : keyevent.which);
        if (code == 9 && $('.bootstrap-select button:focus').length) {
            $("#repos").selectpicker('toggle');
            $('.bootstrap-select .bs-searchbox input').focus();
        }
    });
}

function updateOptions(newOptions) {
    // Skip update if the options are the same, to avoid losing selected state.
    var currentOptions = [];
    $('#repos').find('option').each(function(){
        currentOptions.push($(this).attr('value'));
    });
    if (_.isEqual(currentOptions, newOptions)) {
        return;
    }

    $('#repos').empty();

    newOptions.sort()
    var groups = new Map()
    groups.set('/', $('#repos'))

    var repos = {};
    newOptions.forEach(function(option){
        var group = option.split("/")[0];
        if (repos[group]) {
            repos[group].push(option);
        } else {
            repos[group] = [option];
        };
    });

    for (var key in repos) {
        if (repos.hasOwnProperty(key)) {
            var repo = repos[key];
            if (!groups.has(key)) {
                var groupDOM = $('<optgroup>').attr('label', key);
                $('#repos').append(groupDOM);
                groups.set(key, groupDOM);
            };

            repo.forEach(function(fullPath){
                var path = fullPath.split('/');
                var name = path.slice(path.length - 1, path.length);
                return groups.get(key).append($('<option>').attr('value', fullPath).text(name));
            });
        }
    }

    groups.clear()
    $('#repos').selectpicker('refresh');
}

function updateSelected(newSelected) {
    $('#repos').selectpicker('val', newSelected);
}

module.exports = {
    init: init,
    updateOptions: updateOptions,
    updateSelected: updateSelected
}
