function v = helics_filtertype_custom()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1128095517);
  end
  v = vInitialized;
end
