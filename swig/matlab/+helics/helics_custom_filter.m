function v = helics_custom_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783862);
  end
  v = vInitialized;
end
