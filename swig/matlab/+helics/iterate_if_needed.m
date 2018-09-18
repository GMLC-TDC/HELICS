function v = iterate_if_needed()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107599);
  end
  v = vInitialized;
end
